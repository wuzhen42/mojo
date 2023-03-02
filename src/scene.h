#pragma once

#include "pencil.h"
#include <map>

namespace mojo {
struct Scene {
  int frame = 0;
  std::map<std::string, Pencil> pencils;
};
} // namespace mojo