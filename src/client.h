#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <condition_variable>
#include <thread>
#include <BearLibTerminal.h>
#include <nngpp/nngpp.h>
#include <nngpp/protocol/req0.h>
#include <nngpp/protocol/sub0.h>
#include <nngpp/platform/platform.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#include "lib/date.h"
#pragma GCC diagnostic pop

#define CLIENT "client"

namespace client {

std::vector<char *> messages;
std::mutex messages_mutex;
std::mutex message_buffer_mutex;
std::condition_variable messages_cond_var;
std::condition_variable message_buffer_cond_var;
std::atomic<bool> can_read_from_messages{true};
std::atomic<bool> can_write_to_messages{false};
std::atomic<bool> can_read_from_message_buffer{true};
std::atomic<bool> can_write_to_message_buffer{false};

auto start_reading_from_message_buffer() {
  can_write_to_message_buffer = false;
  std::unique_lock<std::mutex> lock(message_buffer_mutex);
  message_buffer_cond_var.wait(lock, [] { return can_read_from_message_buffer.load(); });
  return lock;
}

auto stop_reading_from_message_buffer(std::unique_lock<std::mutex> lock) {
  can_write_to_message_buffer = true;
  lock.unlock();
  message_buffer_cond_var.notify_one();
}

auto start_writing_to_message_buffer() {
  can_read_from_message_buffer = false;
  std::unique_lock<std::mutex> lock(message_buffer_mutex);
  message_buffer_cond_var.wait(lock, [] { return can_write_to_message_buffer.load(); });
  return lock;
}

auto stop_writing_to_message_buffer(std::unique_lock<std::mutex> lock) {
  can_read_from_message_buffer = true;
  lock.unlock();
  message_buffer_cond_var.notify_one();
}

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

void render(const std::string &message) {
  terminal_clear();
  auto reading_lock{start_reading_from_messages()};
  for (size_t i = 0; i < messages.size(); i++) {
    terminal_print(1, int(18 - i * 3 - 1), messages[i]);
  }
  stop_reading_from_messages(std::move(reading_lock));
  std::stringstream message_text;
  reading_lock = start_reading_from_message_buffer();
  message_text << "> " << message;
  stop_reading_from_message_buffer(std::move(reading_lock));
  for (int i = 0; i < 78; i++) {
    terminal_put(i + 1, 21, '-');
  }
  terminal_print(1, 23, message_text.str().c_str());
  terminal_refresh();
}

void process_input(const std::shared_ptr<bool> &should_exit,
                   std::stringstream *message_buffer,
                   nng::socket *req_sock,
                   const std::vector<int> &printable_chars_) {
  if (terminal_has_input()) {
    auto key{terminal_read()};
    auto reading_lock{start_reading_from_message_buffer()};
    auto message_buffer_str{message_buffer->str()};
    stop_reading_from_message_buffer(std::move(reading_lock));
    if (std::find(printable_chars_.begin(), printable_chars_.end(), key) != printable_chars_.end()
        && message_buffer_str.size() + 2 < 78) {
      auto writing_lock{start_writing_to_message_buffer()};
      (*message_buffer) << char(terminal_state(TK_CHAR));
      stop_writing_to_message_buffer(std::move(writing_lock));
    } else if (key == TK_CLOSE || key == TK_ESCAPE) {
      *should_exit = true;
    } else if (key == TK_BACKSPACE && !message_buffer_str.empty()) {
      auto writing_lock{start_writing_to_message_buffer()};
      message_buffer_str.erase(message_buffer_str.end() - 1);
      message_buffer->str(message_buffer_str);
      stop_writing_to_message_buffer(std::move(writing_lock));
    } else if ((key == TK_ENTER || key == TK_KP_ENTER) && !message_buffer_str.empty()) {
      std::thread generating_send_message_event_thread([req_sock, message_buffer_str, message_buffer]() -> void {
        try {
          std::stringstream message;
          {
            using namespace date;
            message << floor<std::chrono::seconds>(std::chrono::system_clock::now());
          }
          message << std::endl << message_buffer_str;
          auto nng_message{nng::make_msg(0)};
          const auto message_str{message.str()};
          nng_message.body().insert(nng::view(message_str.c_str(), message_str.size()));
          req_sock->send(std::move(nng_message));
          auto writing_lock{start_writing_to_message_buffer()};
          message_buffer->str(std::string());
          stop_writing_to_message_buffer(std::move(writing_lock));
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
      generating_send_message_event_thread.join();
    }
  }
}

auto run(const char *req_rep_endpoint, const char *pub_sub_endpoint) {
  auto req_sock{nng::req::open()};
  std::vector<int> printable_chars{
      TK_A, TK_B, TK_C, TK_D, TK_E, TK_F, TK_G, TK_H, TK_I, TK_J, TK_K, TK_L, TK_M, TK_N, TK_O, TK_P, TK_Q, TK_R,
      TK_S, TK_T, TK_U, TK_V, TK_W, TK_X, TK_Y, TK_Z, TK_1, TK_2, TK_3, TK_4, TK_5, TK_6, TK_7, TK_8, TK_9, TK_0,
      TK_SPACE, TK_MINUS, TK_EQUALS, TK_LBRACKET, TK_RBRACKET, TK_BACKSLASH, TK_SEMICOLON, TK_APOSTROPHE, TK_GRAVE,
      TK_COMMA, TK_PERIOD, TK_SLASH, TK_KP_DIVIDE, TK_KP_MULTIPLY, TK_KP_MINUS, TK_KP_PLUS, TK_KP_1, TK_KP_2,
      TK_KP_3, TK_KP_4, TK_KP_5, TK_KP_6, TK_KP_7, TK_KP_8, TK_KP_9, TK_KP_0, TK_KP_PERIOD
  };
  std::stringstream message_buffer;
  auto should_exit = std::make_shared<bool>(false);
  auto subscribing_thread{std::thread([pub_sub_endpoint, should_exit]() -> void {
    try {
      auto sub_sock{nng::sub::open()};
      sub_sock.set_opt(NNG_OPT_SUB_SUBSCRIBE, {});
      sub_sock.dial(pub_sub_endpoint);
      while (!*should_exit) {
        auto msg{sub_sock.recv_msg()};
        auto message{static_cast<char *>(msg.body().data())};
        auto writing_lock{start_writing_to_messages()};
        messages.insert(messages.begin(), message);
        while (messages.size() > 6) {
          messages.pop_back();
        }
        stop_writing_to_messages(std::move(writing_lock));
      }
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
  terminal_open();
  std::stringstream terminal_options;
  terminal_options << "window: title='nng-blt-demo - client-server chat, mode: " << CLIENT << "';";
  terminal_set(terminal_options.str().c_str());
  req_sock.dial(req_rep_endpoint);
  while (!*should_exit) {
    render(message_buffer.str());
    process_input(should_exit, &message_buffer, &req_sock, printable_chars);
  }
  subscribing_thread.detach();
  terminal_close();
}

}
