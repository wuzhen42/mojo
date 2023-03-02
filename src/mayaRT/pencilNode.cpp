#include "pencilNode.h"

#include <maya/MFnPluginData.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MGlobal.h>
#include <maya/MTime.h>
#include <sstream>

MTypeId MPencilNode::sTypeId(0x49D08);
MString MPencilNode::sTypeName("mojoPencil");
MString MPencilNode::sDrawDbClassification("drawdb/geometry/mojoPencil");
MString MPencilNode::sDrawRegistrantId("mojoPencilDraw");
MObject MPencilNode::sAttrData;
MObject MPencilNode::sAttrTime;

MStatus MPencilNode::initialize() {
  MFnTypedAttribute tAttr;
  MFnUnitAttribute uAttr;
  sAttrData = tAttr.create("data", "data", MPencilData::sId);
  addAttribute(sAttrData);
  attributeAffects(sAttrData, nodeBoundingBox);
  sAttrTime = uAttr.create("time", "time", MTime{});
  addAttribute(sAttrTime);
  attributeAffects(sAttrTime, nodeBoundingBox);
  return MS::kSuccess;
}

void MPencilNode::resetCurrentFrame() { currentFrame = INT_MIN; }

std::optional<mojo::PencilFrame> MPencilNode::fetch() {
  MTime time = MPlug{thisMObject(), sAttrTime}.asMTime();
  int frame = time.as(MTime::uiUnit());
  if (frame == currentFrame)
    return std::nullopt;

  MPlug plug{thisMObject(), sAttrData};
  if (plug.isNull())
    return std::nullopt;

  MObject pencilData;
  plug.getValue(pencilData);
  MFnPluginData pdFn(pencilData);
  const MPencilData *pencil = dynamic_cast<const MPencilData *>(pdFn.constData());
  if (!pencil)
    return std::nullopt;

  currentFrame = frame;
  return pencil->unionFrameAt(frame);
}
