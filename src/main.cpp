#include <exception>
#include <functional>
#include <iostream>
#include <map>
#include <stdexcept>
#include <BearLibTerminal.h>
#include <nngpp/nngpp.h>
#include "client.h"
#include "server.h"

void run(const std::string &mode, const std::string &req_rep_endpoint, const std::string &pub_sub_endpoint) {
  std::map<std::string, void (*)(const char *, const char *)> function_ptrs_for_modes = {
      {CLIENT, &client::run},
      {SERVER, &server::run}
  };
  auto function_ptr = function_ptrs_for_modes.find(mode);
  if (function_ptr == function_ptrs_for_modes.end()) {
    throw std::invalid_argument("Invalid running mode is passed");
  }
  std::stringstream endpoint;
  std::__invoke(function_ptr->second, req_rep_endpoint.c_str(), pub_sub_endpoint.c_str());
}

int main(int argc, char *argv[]) {
  try {
    if (argc < 4) {
      throw std::invalid_argument(
          "Invalid CLI args are passed\n"
          "Usage: <exec_name> <mode> <req_rep_endpoint> <pub_sub_endpoint>\n"
          "Allowed modes: client, server"
      );
    }
    auto mode = std::string(argv[1]);
    if (mode != CLIENT && mode != SERVER) {
      throw std::invalid_argument("Invalid running mode is passed");
    }
    auto req_rep_endpoint = std::string(argv[2]);
    auto pub_sub_endpoint = std::string(argv[3]);
    run(mode, req_rep_endpoint, pub_sub_endpoint);
  } catch (nng::exception &e) {
    terminal_close();
    std::cerr << e.who() << ": " << nng::to_string(e.get_error()) << std::endl;
    return EXIT_FAILURE;
  } catch (std::exception &e) {
    terminal_close();
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
