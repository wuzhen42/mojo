#include "mayaConnector.h"
#include "pencilData.h"
#include "pencilNode.h"
#include "protocal.hpp"
#include <boost/algorithm/string.hpp>
#include <capnp/serialize.h>

#include <maya/MDagModifier.h>
#include <maya/MDagPath.h>
#include <maya/MFnDagNode.h>
#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>

void MayaConnector::send_maya(std::string path) {
  MSelectionList selection;
  MGlobal::getSelectionListByName(path.c_str(), selection);
  if (selection.isEmpty())
    return;

  MDagPath dagPath;
  selection.getDagPath(0, dagPath);

  MFnDagNode dagNode(dagPath);
  if (dagNode.typeId() == MPencilNode::sTypeId) {
    MPlug plug = dagNode.findPlug("data", true);
    send_pencil(path, static_cast<MPencilData *>(plug.asMDataHandle().asPluginData())->get());
  }
}

void MayaConnector::on_pencil_change(std::string path, mojo::Pencil pencil) {
  std::replace(path.begin(), path.end(), '/', '|');

  MObject objPencilNode;
  MSelectionList selection;
  if (MGlobal::getSelectionListByName(path.c_str(), selection) && !selection.isEmpty()) {
    CHECK_MSTATUS(selection.getDependNode(0, objPencilNode));
    if (MFnDependencyNode(objPencilNode).typeId() != MPencilNode::sTypeId)
      return log(LogLevel::Error, "object {} already exists and is not {}", path, MPencilNode::sTypeName.asChar());
  } else {
    MDagModifier modifier;
    int beg = 1;
    MObject parent = MObject::kNullObj;
    while (beg != std::string::npos) {
      int end = path.find('|', beg);
      MSelectionList selection;
      MString currentPath = path.substr(0, end).c_str();
      if (!MGlobal::getSelectionListByName(currentPath, selection) && selection.isEmpty()) { // not exist, need to create
        // find parent
        if (beg > 0 && parent.isNull()) {
          MString parentPath = path.substr(0, beg - 1).c_str();
          MGlobal::getSelectionListByName(parentPath, selection);
          selection.getDependNode(0, parent);
        }

        MString name = path.substr(beg, end - beg).c_str();
        if (end != std::string::npos) {
          parent = modifier.createNode("transform", parent);
          modifier.renameNode(parent, name);
        } else {
          MPlug plugTime;
          MGlobal::getSelectionListByName("time1.outTime", selection);
          selection.getPlug(0, plugTime);
          objPencilNode = modifier.createNode(MPencilNode::sTypeId, parent);
          modifier.renameNode(objPencilNode, name);
          CHECK_MSTATUS(modifier.connect(plugTime, MFnDependencyNode(objPencilNode).findPlug("time", true)));
          break;
        }
      }
      beg = (end == std::string::npos ? end : end + 1);
    }
    modifier.doIt();
  }
  if (objPencilNode.isNull())
    return log(LogLevel::Error, "{} object is null", path);

  MPencilNode *node = dynamic_cast<MPencilNode *>(MFnDependencyNode(objPencilNode).userNode());
  if (!node)
    return log(LogLevel::Error, "{} is invalid", path);

  node->resetCurrentFrame();
  MPlug plug = MFnDependencyNode(objPencilNode).findPlug("data", true);
  CHECK_MSTATUS(plug.setMPxData(new MPencilData{std::move(pencil)}));
}
