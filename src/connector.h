#pragma once

#include "pencil.h"
#include "protocal.hpp"
#include <exception>
#include <zmqpp/zmqpp.hpp>

class Connector {
  zmqpp::context context;
  zmqpp::socket sub, pub;

  // (connected, port)
  std::pair<bool, unsigned> port_publish;
  LogLevel log_level = LogLevel::Info;

  struct IORecord {
    TimePoint time;
    Message type;
  } last_recv;

public:
  Connector();

  virtual ~Connector() = default;

  void try_recv();

  void send_frame(int frame);

  void send_pencil(std::string path, const mojo::Pencil &pencil);

protected:
  virtual std::string topic() { throw std::runtime_error("topic() Not Implemented"); };

  virtual void on_frame_change(int frame) {}

  virtual void on_pencil_change(std::string path, mojo::Pencil pencil) {}

  virtual void log(std::string text) const;

  virtual void on_recv(zmqpp::message msg) {}

  template <typename... _Types> void log(LogLevel level, std::string_view fmt, _Types &&..._Args) const {
    print(level, std::vformat(fmt, std::make_format_args(_Args...)));
  }

private:
  void send(zmqpp::message msg);

  void try_join();

  void print(LogLevel level, std::string text) const;

  void send_alive();
};
