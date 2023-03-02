#pragma once

#include <GL/glew.h>

#include "pencilNode.h"
#include <maya/MFnDependencyNode.h>
#include <maya/MGlobal.h>
#include <maya/MHWGeometryUtilities.h>
#include <maya/MPxDrawOverride.h>

class MPencilDraw : public MPxDrawOverride {
  MPencilNode *node;

  struct PencilDrawData : public MUserData {
    PencilDrawData() : MUserData(false) {}

    struct {
      bool active = false;
      GLuint VAO = NULL;
      GLuint VBO_position = NULL;
      GLuint VBO_thickness = NULL;
      GLuint VBO_color = NULL;
      GLuint VBO_depth = NULL;
      GLuint EBO_lines = NULL;
      GLuint program = NULL;
      GLuint UNI_MVP = NULL;
      GLuint UNI_projection = NULL;
      GLuint UNI_pixelSize = NULL;
      GLuint UNI_viewport = NULL;
      GLuint UNI_displayStatus = NULL;
      unsigned drawSize = 0;
    } stroke;

    struct {
      bool active = false;
      GLuint VAO = NULL;
      GLuint VBO_position = NULL;
      GLuint VBO_color = NULL;
      GLuint VBO_depth = NULL;
      GLuint EBO_triangles = NULL;
      GLuint program = NULL;
      GLuint UNI_MVP = NULL;
      unsigned drawSize = 0;
    } filled;

    unsigned displayStatus = MHWRender::DisplayStatus::kDormant;
  } data;

public:
  static MPxDrawOverride *create(const MObject &obj) { return new MPencilDraw(obj); }

  MHWRender::DrawAPI supportedDrawAPIs() const override { return MHWRender::kOpenGLCoreProfile; }

  MUserData *prepareForDraw(const MDagPath &objPath, const MDagPath &cameraPath, const MHWRender::MFrameContext &frameContext, MUserData *oldData) override;

  static void draw(const MHWRender::MDrawContext &context, const MUserData *data);

  bool hasUIDrawables() const override { return false; }

private:
  MPencilDraw(const MObject &obj);

  ~MPencilDraw() override;

  PencilDrawData *decorate(MUserData *_data, const MDagPath &objPath);
};
