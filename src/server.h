#include <cstdio>
#include <iostream>
#include <condition_variable>
#include <vector>
#include <functional>
#include <thread>
#include <BearLibTerminal.h>
#include <nngpp/nngpp.h>
#include <nngpp/protocol/rep0.h>
#include <nngpp/protocol/pub0.h>
#include <nngpp/platform/platform.h>

// TODO: this value probably should be CLI arg
#ifndef PARALLEL_WORKS_COUNT
#define PARALLEL_WORKS_COUNT 128
#endif

#define SERVER "server"

namespace server {

std::vector<char *> messages;
std::mutex messages_mutex;
std::condition_variable messages_cond_var;
std::atomic<bool> can_read_from_messages{true};
std::atomic<bool> can_write_to_messages{false};

auto start_reading_from_messages() {
  can_write_to_messages = false;
  std::unique_lock<std::mutex> lock(messages_mutex);
  messages_cond_var.wait(lock, [] { return can_read_from_messages.load(); });
  return lock;
}

auto stop_reading_from_messages(std::unique_lock<std::mutex> lock) {
  can_write_to_messages = true;
  lock.unlock();
  messages_cond_var.notify_one();
}

auto start_writing_to_messages() {
  can_read_from_messages = false;
  std::unique_lock<std::mutex> lock(messages_mutex);
  messages_cond_var.wait(lock, [] { return can_write_to_messages.load(); });
  return lock;
}

auto stop_writing_to_messages(std::unique_lock<std::mutex> lock) {
  can_read_from_messages = true;
  lock.unlock();
  messages_cond_var.notify_one();
}

void server_callback(void *arg);

struct Work {
  enum {
    INIT,
    RECV,
    WAIT,
    SEND
  } state = INIT;
  nng::aio aio{server_callback, this};
  nng::msg msg;
  nng::ctx ctx;
  nng::socket *pub_sock;

  explicit Work(nng::socket_view sock, nng::socket *pub_sock_) : ctx{sock}, pub_sock{pub_sock_} {}
};

void server_callback(void *arg) {
  try {
    auto work{static_cast<Work *>(arg)};
    switch (work->state) {
      case Work::INIT: {
        work->state = Work::RECV;
        work->ctx.recv(work->aio);
        break;
      }
      case Work::RECV: {
        auto result{work->aio.result()};
        if (result != nng::error::success) {
          throw nng::exception(result);
        }
        auto msg{work->aio.release_msg()};
        try {
          auto message{static_cast<char *>(msg.body().data())};
          auto writing_lock{start_writing_to_messages()};
          messages.insert(messages.begin(), message);
          while (messages.size() > 8) {
            messages.pop_back();
          }
          stop_writing_to_messages(std::move(writing_lock));
          auto publishing_thread{std::thread([work, message]() -> void {
            try {
              auto nng_message{nng::make_msg(0)};
              nng_message.body().insert(nng::view(message, std::string(message).size()));
              work->pub_sock->send(std::move(nng_message));
            } catch (nng::exception &e) {
              terminal_close();
              std::cerr << e.who() << ": " << nng::to_string(e.get_error()) << std::endl;
              exit(EXIT_FAILURE);
            } catch (std::exception &e) {
              terminal_close();
              std::cerr << e.what() << std::endl;
              exit(EXIT_FAILURE);
            }
          })};
          publishing_thread.detach();
        } catch (nng::exception &) {
          work->ctx.recv(work->aio);
          return;
        }
        work->msg = std::move(msg);
        work->state = Work::WAIT;
        break;
      }
      case Work::WAIT: {
        work->aio.set_msg(std::move(work->msg));
        work->state = Work::SEND;
        work->ctx.send(work->aio);
        break;
      }
      case Work::SEND: {
        auto result{work->aio.result()};
        if (result != nng::error::success) {
          throw nng::exception(result);
        }
        work->state = Work::RECV;
        work->ctx.recv(work->aio);
        break;
      }
      default: {
        throw nng::exception(nng::error::state);
      }
    }
  } catch (nng::exception &e) {
    if (e.get_error() == static_cast<nng::error>(NNG_ECLOSED)) {
      // this error means that socket just was closed, let's ignore it...
    } else {
      throw e;
    }
  }

}

auto render() {
  terminal_clear();
  auto reading_lock{start_reading_from_messages()};
  for (size_t i = 0; i < messages.size(); i++) {
    terminal_print(1, int(23 - i * 3 - 1), messages[i]);
  }
  stop_reading_from_messages(std::move(reading_lock));
  terminal_refresh();
}

auto process_input(const std::shared_ptr<bool> &should_exit) {
  if (terminal_has_input()) {
    auto key{terminal_read()};
    if (key == TK_CLOSE || key == TK_ESCAPE) {
      *should_exit = true;
    }
  }
}

void run(const char *req_rep_endpoint, const char *pub_sub_endpoint) {
  std::vector<std::unique_ptr<Work>> works;
  auto should_exit = std::make_shared<bool>(false);
  works.reserve(PARALLEL_WORKS_COUNT);
  auto rep_sock{nng::rep::open()};
  auto pub_sock{nng::pub::open()};
  pub_sock.listen(pub_sub_endpoint);
  for (int i = 0; i < PARALLEL_WORKS_COUNT; ++i) {
    works.push_back(std::make_unique<Work>(rep_sock, &pub_sock));
  }
  auto listening_thread = std::thread([&rep_sock, req_rep_endpoint, &works, should_exit]() -> void {
    try {
      rep_sock.listen(req_rep_endpoint);
      for (int i = 0; i < PARALLEL_WORKS_COUNT; i++) {
        server_callback(works.at(i).get());
      }
      while (!*should_exit);
    } catch (nng::exception &e) {
      terminal_close();
      std::cerr << e.who() << ": " << nng::to_string(e.get_error()) << std::endl;
      exit(EXIT_FAILURE);
    } catch (std::exception &e) {
      terminal_close();
      std::cerr << e.what() << std::endl;
      exit(EXIT_FAILURE);
    }
  });
  terminal_open();
  std::stringstream terminal_options;
  terminal_options << "window: title='nng-blt-demo - client-server chat, mode: " << SERVER << "';";
  terminal_set(terminal_options.str().c_str());
  while (!*should_exit) {
    render();
    process_input(should_exit);
  }
  listening_thread.join();
  terminal_close();
}

}
