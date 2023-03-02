#pragma once

#include "protocal.hpp"
#include "scene.h"
#include <format>
#include <set>
#include <zmqpp/zmqpp.hpp>

namespace mojo {
class Server {
  Scene scene;

  zmqpp::context context;
  zmqpp::socket publish, host;
  std::map<int, TimePoint> listens;

public:
  Server();

  bool subscribe(int port, const std::string &topic);

  void send(zmqpp::message msg);

  void try_recv();

  void check_alive();

  bool zero_client() const;

private:
  void print(LogLevel level, std::string text) const;

  template <typename... _Types> void log(LogLevel level, std::string_view fmt, _Types &&..._Args) {
    print(level, std::vformat(fmt, std::make_format_args(_Args...)));
  }
};

} // namespace mojo
