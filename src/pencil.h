#pragma once

#include "mojo.capnp.h"
#include <capnp/compat/json.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace mojo {
struct PencilFrame {
  struct Point {
    glm::vec3 position;
    float thickness;
  };

  struct Stroke {
    glm::vec4 color;
    std::vector<Point> points;
    std::vector<unsigned> triangles;

    bool fill() const { return !triangles.empty(); }
  };

  int frame;
  std::vector<Stroke> strokes;

  void add_stroke(std::array<float, 4> color, std::vector<float> points);

  void add_filled(std::array<float, 4> color, std::vector<float> points, std::vector<unsigned> indices);
};

struct PencilLayer {
  std::string name;
  std::vector<PencilFrame> frames;

  void push_frame(PencilFrame frame);
};

struct Pencil {
  std::vector<PencilLayer> layers;

  void push_layer(PencilLayer layer);

  kj::Array<capnp::word> to_words() const;

  void set(proto::Pencil::Reader msg);

  void build(capnp::MallocMessageBuilder &builder) const;

  std::string to_json() const;

  std::vector<PencilLayer>::const_iterator begin() const { return layers.begin(); }

  std::vector<PencilLayer>::const_iterator end() const { return layers.end(); }
};
} // namespace mojo
