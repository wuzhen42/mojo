#include "pencil.h"
#include <array>
#include <capnp/serialize.h>

namespace mojo {
void Pencil::push_layer(PencilLayer layer) { layers.emplace_back(std::move(layer)); }

kj::Array<capnp::word> Pencil::to_words() const {
  capnp::MallocMessageBuilder builder;
  build(builder);
  return capnp::messageToFlatArray(builder);
}

void Pencil::set(proto::Pencil::Reader capPencil) {
  layers.clear();

  for (auto capLayer : capPencil.getLayers()) {
    PencilLayer layer;
    layer.name = capLayer.getName();
    for (auto capFrame : capLayer.getFrames()) {
      PencilFrame frame;
      frame.frame = capFrame.getFrame();
      for (auto capStroke : capFrame.getStrokes()) {
        PencilFrame::Stroke stroke;
        const auto &capColor = capStroke.getColor();
        stroke.color = {capColor.getR(), capColor.getG(), capColor.getB(), capColor.getA()};
        if (capStroke.hasTriangles()) {
          for (uint32_t index : capStroke.getTriangles())
            stroke.triangles.push_back(index);
        }
        for (auto capPoint : capStroke.getPoints()) {
          PencilFrame::Point point;
          auto capPosition = capPoint.getPosition();
          point.position = {capPosition.getX(), capPosition.getY(), capPosition.getZ()};
          point.thickness = capPoint.getPressure();
          stroke.points.push_back(point);
        }
        frame.strokes.emplace_back(std::move(stroke));
      }
      layer.frames.emplace_back(std::move(frame));
    }
    layers.emplace_back(std::move(layer));
  }
}

void Pencil::build(capnp::MallocMessageBuilder &builder) const {
  proto::Pencil::Builder capPencil = builder.initRoot<proto::Pencil>();

  auto builderLayers = capPencil.initLayers(layers.size());
  for (int idxLayer = 0; idxLayer != layers.size(); ++idxLayer) {
    const auto &layer = layers[idxLayer];
    auto builderLayer = builderLayers[idxLayer];
    builderLayer.setName(layer.name);

    auto builderFrames = builderLayer.initFrames(layer.frames.size());
    for (int idxFrame = 0; idxFrame != layer.frames.size(); ++idxFrame) {
      const auto &frame = layer.frames[idxFrame];
      auto builderFrame = builderFrames[idxFrame];
      builderFrame.setFrame(frame.frame);

      auto builderStrokes = builderFrame.initStrokes(frame.strokes.size());
      for (int idxStroke = 0; idxStroke != frame.strokes.size(); ++idxStroke) {
        const auto &stroke = frame.strokes[idxStroke];
        auto builderStroke = builderStrokes[idxStroke];
        auto builderColor = builderStroke.getColor();
        builderColor.setR(stroke.color.r);
        builderColor.setG(stroke.color.g);
        builderColor.setB(stroke.color.b);
        builderColor.setA(stroke.color.a);
        if (stroke.fill()) {
          auto builderTriangles = builderStroke.initTriangles(stroke.triangles.size());
          for (int i = 0; i != stroke.triangles.size(); ++i)
            builderTriangles.set(i, stroke.triangles[i]);
        }

        auto builderPoints = builderStroke.initPoints(stroke.points.size());
        for (int idxPoint = 0; idxPoint != stroke.points.size(); ++idxPoint) {
          auto point = stroke.points[idxPoint];
          auto builderPoint = builderPoints[idxPoint];
          auto builderPosition = builderPoint.getPosition();
          builderPosition.setX(point.position.x);
          builderPosition.setY(point.position.y);
          builderPosition.setZ(point.position.z);
          builderPoint.setPressure(point.thickness);
        }
      }
    }
  }
}

std::string Pencil::to_json() const {
  capnp::MallocMessageBuilder builder;
  this->build(builder);
  capnp::JsonCodec codec;
  return codec.encode(builder.getRoot<proto::Pencil>().asReader()).cStr();
}

void PencilLayer::push_frame(PencilFrame frame) { frames.emplace_back(std::move(frame)); }

void PencilFrame::add_stroke(std::array<float, 4> color, std::vector<float> points) {
  Stroke stroke;
  stroke.color = {color[0], color[1], color[2], color[3]};
  unsigned num_points = points.size() / 4;
  for (int i = 0; i != num_points; ++i) {
    glm::vec3 pos{points[4 * i], points[4 * i + 1], points[4 * i + 2]};
    stroke.points.push_back(Point{pos, points[4 * i + 3]});
  }
  strokes.push_back(stroke);
}

void PencilFrame::add_filled(std::array<float, 4> color, std::vector<float> points, std::vector<unsigned> indices) {
  add_stroke(color, points);
  strokes.back().triangles = indices;
}
} // namespace mojo