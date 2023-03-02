#include "protocal.hpp"
#include "server.h"

int main(int argc, char *argv[]) {
  mojo::Server server;

  TimePoint lastActiveTime = currentTime();
  while (true) {
    server.try_recv();

    if (!server.zero_client())
      lastActiveTime = currentTime();
    if (std::chrono::duration_cast<std::chrono::seconds>(currentTime() - lastActiveTime).count() >= 5)
      break;
  }
}