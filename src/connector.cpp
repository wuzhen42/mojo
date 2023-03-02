#include "connector.h"
#include "protocal.hpp"
#include "socket_utils.hpp"
#include <capnp/serialize.h>
#include <format>

Connector::Connector() : sub(context, zmqpp::socket_type::sub), pub(context, zmqpp::socket_type::pub) {
  sub.connect(std::format("tcp://localhost:{}", PORT_PUBLISH));
  sub.subscribe("mojo");

  pub.bind("tcp://*:0");
  port_publish.first = false;
  port_publish.second = mojo::get_port(pub.get<std::string>(zmqpp::socket_option::last_endpoint));
  pub.close();
  pub = zmqpp::socket{context, zmqpp::socket_type::pub};
  pub.bind(std::format("tcp://*:{}", PORT_HOST));
}

void Connector::log(std::string text) const { std::cout << text << std::endl; }

void Connector::send(zmqpp::message msg) {
  if (msg.get<int>(0) == last_recv.type && std::chrono::duration_cast<std::chrono::milliseconds>(currentTime() - last_recv.time).count() < 100)
    return;

  msg.push_front(this->port_publish.second);
  msg.push_front(this->topic());
  pub.send(msg);
}

void Connector::try_join() {
  zmqpp::message msg;
  msg << "mojo" << port_publish.second << Message::CONNECT << this->topic();
  pub.send(msg);
}

void Connector::try_recv() {
  if (!port_publish.first)
    try_join();
  else
    send_alive();

  zmqpp::message message;
  if (!sub.receive(message, true))
    return;

  int publisher = message.get<int>(message.next());
  if (publisher == port_publish.second)
    return;
  Message header = static_cast<Message>(message.get<int>(message.next()));

  last_recv.time = currentTime();
  last_recv.type = header;

  if (header == Message::CONNECT) {
    int port = message.get<int>(message.next());
    if (!port_publish.first && port == port_publish.second) {
      log(LogLevel::Info, "connected to mojo");
      pub.close();
      pub = zmqpp::socket{context, zmqpp::socket_type::pub};
      pub.bind(std::format("tcp://*:{}", port_publish.second));
      port_publish.first = true;
    }
  } else if (header == Message::FRAME) {
    int frame = message.get<int>(message.next());
    this->on_frame_change(frame);
  } else if (header == Message::PENCIL) {
    std::string path = message.get<std::string>(message.next());

    std::size_t part = message.next();
    kj::Array<capnp::word> words = kj::heapArray<capnp::word>(message.size(part) / sizeof(capnp::word));
    std::memcpy(words.begin(), message.raw_data(part), message.size(part));

    capnp::FlatArrayMessageReader flatArray(words);
    mojo::Pencil pencil;
    pencil.set(flatArray.getRoot<proto::Pencil>());
    on_pencil_change(path, std::move(pencil));
  } else {
    on_recv(std::move(message));
  }
}

void Connector::send_frame(int frame) {
  zmqpp::message msg;
  msg << Message::FRAME << frame;
  send(std::move(msg));
}

void Connector::send_pencil(std::string path, const mojo::Pencil &pencil) {
  zmqpp::message msg;
  kj::Array<capnp::word> words = pencil.to_words();
  msg << Message::PENCIL << path;
  msg.push_back(words.begin(), words.size() * sizeof(words[0]));
  send(std::move(msg));
}

void Connector::print(LogLevel level, std::string text) const {
  if (level > this->log_level)
    return;
  log(text);
}

void Connector::send_alive() {
  zmqpp::message message;
  message << Message::HEARTBEAT;
  send(std::move(message));
}
