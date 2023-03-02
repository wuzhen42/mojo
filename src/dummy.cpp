#include "connector.h"

class DummyConnector : public Connector {
  std::string topic() override { return "dummy"; }
};

int main() {
  DummyConnector connector{};
  while (true) {
    connector.try_recv();
  }
}