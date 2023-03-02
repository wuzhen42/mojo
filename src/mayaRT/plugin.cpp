#include <GL/glew.h>

#include "pencilData.h"
#include "pencilDraw.h"
#include "pencilNode.h"
#include <maya/MDrawRegistry.h>
#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>

MStatus initializePlugin(MObject &obj) {
  if (GLEW_OK != glewInit())
    return MS::kFailure;

  MFnPlugin plugin(obj, "OctMedia", "1.0", "2023");
  plugin.registerData(MPencilData::sName, MPencilData::sId, MPencilData::create, MPxData::kData);
  plugin.registerNode(MPencilNode::sTypeName, MPencilNode::sTypeId, MPencilNode::create, MPencilNode::initialize, MPxNode::kLocatorNode,
                      &MPencilNode::sDrawDbClassification);
  MHWRender::MDrawRegistry::registerDrawOverrideCreator(MPencilNode::sDrawDbClassification, MPencilNode::sDrawRegistrantId, MPencilDraw::create);

  MGlobal::executePythonCommand("from mojo import shelf; shelf.on_load()");
  return MS::kSuccess;
}

MStatus uninitializePlugin(MObject &obj) {
  MFnPlugin plugin(obj);
  MGlobal::executePythonCommand("from mojo import shelf; shelf.on_unload()");
  MHWRender::MDrawRegistry::deregisterDrawOverrideCreator(MPencilNode::sDrawDbClassification, MPencilNode::sDrawRegistrantId);
  plugin.deregisterNode(MPencilNode::sTypeId);
  plugin.deregisterData(MPencilData::sId);
  return MS::kSuccess;
}
