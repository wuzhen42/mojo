#include "server.h"
#include <capnp/serialize.h>
#include <format>

#include "socket_utils.hpp"

namespace mojo {
Server::Server() : publish(context, zmqpp::socket_type::pub), host(context, zmqpp::socket_type::sub) {
  publish.bind(std::format("tcp://*:{}", PORT_PUBLISH));
  host.subscribe("");
  subscribe(PORT_HOST, "mojo");
}

bool Server::subscribe(int port, const std::string &topic) {
  if (listens.count(port))
    return false;
  listens[port] = currentTime();
  host.connect(std::format("tcp://localhost:{}", port));
  log(LogLevel::Info, "new {}({}) joined", topic, port);
  return true;
}

void Server::send(zmqpp::message msg) {
  msg.push_front("mojo");
  publish.send(msg);
}

void Server::try_recv() {
  zmqpp::message message;
  if (!host.receive(message, true)) {
    check_alive();
    return;
  }

  int publisher = message.get<int>(message.next());
  int header = message.get<int>(message.next());

  if (header == Message::CONNECT) {
    std::string client = message.get<std::string>(message.next());
    subscribe(publisher, client);
    zmqpp::message msg;
    msg << "mojo" << PORT_PUBLISH << Message::CONNECT << publisher;
    publish.send(msg);
    return;
  }

  if (header == Message::HEARTBEAT) {
    if (listens.count(publisher))
      listens[publisher] = currentTime();
    return;
  }

  if (header == Message::FRAME) {
    int frame = message.get<int>(message.next());
    scene.frame = frame;
  } else if (header == Message::PENCIL) {
    std::string path = message.get<std::string>(message.next());
    log(LogLevel::Debug, "receive pencil {}", path);

    std::size_t part = message.next();
    kj::Array<capnp::word> words = kj::heapArray<capnp::word>(message.size(part) / sizeof(capnp::word));
    std::memcpy(words.begin(), message.raw_data(part), message.size(part));

    capnp::FlatArrayMessageReader flatArray(words);
    proto::Pencil::Reader pencilReader = flatArray.getRoot<proto::Pencil>();
    scene.pencils[path].set(pencilReader);
  } else {
    publish.send(message);
    return;
  }
  message.pop_front();
  send(std::move(message));
}

void Server::check_alive() {
  if (listens.empty())
    return;

  auto iter = listens.begin();
  // skip first client(mojo itself)
  iter++;

  while (iter != listens.end()) {
    if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime() - iter->second).count() > 1000) {
      log(LogLevel::Info, "client {} lost", iter->first);
      iter = listens.erase(iter);
    } else {
      iter++;
    }
  }
}

bool Server::zero_client() const { return listens.size() <= 1; }

void Server::print(LogLevel level, std::string text) const {
  if (level > LogLevel::Info)
    return;
  std::cout << text << std::endl;
}

} // namespace mojo