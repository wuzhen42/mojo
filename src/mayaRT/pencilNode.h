#pragma once

#include "pencilData.h"
#include <maya/MPxLocatorNode.h>
#include <optional>

class MPencilNode : public MPxLocatorNode {
  int currentFrame = INT_MIN;

public:
  static MStatus initialize();

  static void *create() { return new MPencilNode(); }

  void resetCurrentFrame();

  std::optional<mojo::PencilFrame> fetch();

  static MTypeId sTypeId;
  static MString sTypeName;
  static MString sDrawDbClassification;
  static MString sDrawRegistrantId;
  static MObject sAttrData;
  static MObject sAttrTime;

private:
  MPencilNode() = default;
};
