#pragma once
#include <string>

namespace mojo {
inline int get_port(const std::string &endpoint) {
  return std::stoi(endpoint.substr(endpoint.find_last_of(':') + 1));
}
} // namespace mojo