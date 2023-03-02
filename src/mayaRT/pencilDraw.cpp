#include "pencilDraw.h"
#include <filesystem>
#include <format>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <maya/MDrawContext.h>
#include <maya/MGlobal.h>

constexpr unsigned cPrimitiveRestartIndex = 0xFFFF;

void get_stroke_frame_buffer(const mojo::PencilFrame &frame, std::vector<glm::vec3> &position, std::vector<float> &thickness, std::vector<glm::vec4> &colors,
                             std::vector<float> &depths, std::vector<unsigned> &indices) {
  using Vertex = mojo::PencilFrame::Point;
  auto append_point_to_line_strip = [&](Vertex pnt, glm::vec4 color, float depth) {
    indices.push_back(position.size());
    position.push_back(pnt.position);
    thickness.push_back(pnt.thickness);
    colors.push_back(color);
    depths.push_back(depth);
  };

  for (int i = 0; i != frame.strokes.size(); ++i) {
    const auto &stroke = frame.strokes[i];
    if (stroke.fill() || stroke.points.size() < 2)
      continue;

    float depth = i;
    // left extra point for GL_LINE_STRIP_ADJACENCY
    Vertex second = stroke.points[1];
    append_point_to_line_strip(second, stroke.color, depth);

    for (Vertex vtx : stroke.points)
      append_point_to_line_strip(vtx, stroke.color, depth);

    // right extra point for GL_LINE_STRIP_ADJACENCY
    Vertex rsecond = *(stroke.points.rbegin() + 1);
    append_point_to_line_strip(rsecond, stroke.color, depth);

    // stroke end mark
    indices.push_back(cPrimitiveRestartIndex);
  }
}

void get_filled_frame_buffer(const mojo::PencilFrame &frame, std::vector<glm::vec3> &position, std::vector<glm::vec4> &colors, std::vector<float> &depths,
                             std::vector<unsigned> &indices) {
  using Vertex = mojo::PencilFrame::Point;

  auto append_point = [&](glm::vec3 pos, glm::vec4 color, float depth) {
    position.push_back(pos);
    depths.push_back(depth);
    colors.push_back(color);
  };

  for (int i = 0; i != frame.strokes.size(); ++i) {
    const auto &stroke = frame.strokes[i];
    if (!stroke.fill() || stroke.points.size() < 2)
      continue;

    float depth = i;
    const unsigned offset = position.size();
    for (auto pnt : stroke.points)
      append_point(pnt.position, stroke.color, depth);
    for (unsigned index : stroke.triangles)
      indices.push_back(offset + index);
  }
}

std::string readAll(std::filesystem::path path) {
  std::ifstream fin{path};
  if (!fin.is_open())
    throw std::runtime_error(std::format("can't open file '{}'", path.string()));
  return std::string{std::istreambuf_iterator<char>(fin), std::istreambuf_iterator<char>()};
}

GLint compileShader(std::filesystem::path path, GLenum type) {
  const std::string text = readAll(path);
  const char *shaderSource = text.c_str();
  GLint success;
  int shader = glCreateShader(type);
  glShaderSource(shader, 1, &shaderSource, nullptr);
  glCompileShader(shader);
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (GL_FALSE == success) {
    char errorMessage[512];
    glGetShaderInfoLog(shader, 512, NULL, errorMessage);
    throw std::runtime_error(std::format("can't compile shader '{}'. Error: {}", path.string(), errorMessage));
  }
  return shader;
}

MPencilDraw::MPencilDraw(const MObject &obj) : MPxDrawOverride(obj, draw, true) {
  node = dynamic_cast<MPencilNode *>(MFnDependencyNode(obj).userNode());

  try {
    std::filesystem::path path_shader{std::getenv("MOJO_SHADERS")};

    // stroke program
    {
      glGenVertexArrays(1, &data.stroke.VAO);
      GLint vertexShader = compileShader(path_shader / "stroke.vert", GL_VERTEX_SHADER);
      GLint geometryShader = compileShader(path_shader / "stroke.geom", GL_GEOMETRY_SHADER);
      GLint fragmentShader = compileShader(path_shader / "stroke.frag", GL_FRAGMENT_SHADER);

      data.stroke.program = glCreateProgram();
      glAttachShader(data.stroke.program, vertexShader);
      glAttachShader(data.stroke.program, geometryShader);
      glAttachShader(data.stroke.program, fragmentShader);
      glLinkProgram(data.stroke.program);
      {
        int success;
        glGetProgramiv(data.stroke.program, GL_LINK_STATUS, &success);
        if (GL_FALSE == success)
          throw std::runtime_error("can't link shader program");
      }

      data.stroke.UNI_MVP = glGetUniformLocation(data.stroke.program, "MVP");
      data.stroke.UNI_projection = glGetUniformLocation(data.stroke.program, "projection");
      data.stroke.UNI_pixelSize = glGetUniformLocation(data.stroke.program, "pixelsize");
      data.stroke.UNI_viewport = glGetUniformLocation(data.stroke.program, "viewport");
      data.stroke.UNI_displayStatus = glGetUniformLocation(data.stroke.program, "displayStatus");

      glDeleteShader(vertexShader);
      glDeleteShader(geometryShader);
      glDeleteShader(fragmentShader);
    }

    // filled program
    {
      glGenVertexArrays(1, &data.filled.VAO);
      GLint vertexShader = compileShader(path_shader / "fill.vert", GL_VERTEX_SHADER);
      GLint fragmentShader = compileShader(path_shader / "fill.frag", GL_FRAGMENT_SHADER);

      data.filled.program = glCreateProgram();
      glAttachShader(data.filled.program, vertexShader);
      glAttachShader(data.filled.program, fragmentShader);
      glLinkProgram(data.filled.program);
      {
        int success;
        glGetProgramiv(data.filled.program, GL_LINK_STATUS, &success);
        if (GL_FALSE == success)
          throw std::runtime_error("can't link shader program");
      }

      data.filled.UNI_MVP = glGetUniformLocation(data.filled.program, "MVP");

      glDeleteShader(vertexShader);
      glDeleteShader(fragmentShader);
    }
  } catch (const std::exception &e) {
    MGlobal::displayError(std::format("[mojo]error: {}", e.what()).c_str());
  }
}

MPencilDraw::~MPencilDraw() {
  glDeleteVertexArrays(1, &data.stroke.VAO);
  glDeleteBuffers(1, &data.stroke.VBO_position);
  glDeleteBuffers(1, &data.stroke.VBO_thickness);
  glDeleteBuffers(1, &data.stroke.VBO_color);
  glDeleteBuffers(1, &data.stroke.VBO_depth);
  glDeleteBuffers(1, &data.stroke.EBO_lines);
  glDeleteProgram(data.stroke.program);

  glDeleteVertexArrays(1, &data.filled.VAO);
  glDeleteBuffers(1, &data.filled.VBO_position);
  glDeleteBuffers(1, &data.filled.VBO_depth);
  glDeleteBuffers(1, &data.filled.VBO_color);
  glDeleteBuffers(1, &data.filled.EBO_triangles);
  glDeleteProgram(data.filled.program);
}

MPencilDraw::PencilDrawData *MPencilDraw::decorate(MUserData *_data, const MDagPath &objPath) {
  auto data = dynamic_cast<PencilDrawData *>(_data);
  if (!data)
    return nullptr;
  data->displayStatus = MGeometryUtilities::displayStatus(objPath);
  return data;
}

MUserData *MPencilDraw::prepareForDraw(const MDagPath &objPath, const MDagPath &cameraPath, const MHWRender::MFrameContext &frameContext, MUserData *oldData) {
  std::optional<mojo::PencilFrame> result = node->fetch();
  if (!result.has_value())
    return decorate(oldData, objPath);

  // stroke data
  {
    std::vector<glm::vec3> position;
    std::vector<float> thickness, depths;
    std::vector<glm::vec4> colors;
    std::vector<unsigned> indices;
    get_stroke_frame_buffer(result.value(), position, thickness, colors, depths, indices);
    data.stroke.active = !indices.empty();
    if (data.stroke.active) {
      const unsigned numVertices = position.size();

      if (data.stroke.VBO_position != NULL)
        glDeleteBuffers(1, &data.stroke.VBO_position);
      if (data.stroke.VBO_thickness != NULL)
        glDeleteBuffers(1, &data.stroke.VBO_thickness);
      if (data.stroke.VBO_color != NULL)
        glDeleteBuffers(1, &data.stroke.VBO_color);
      if (data.stroke.VBO_depth != NULL)
        glDeleteBuffers(1, &data.stroke.VBO_depth);
      if (data.stroke.EBO_lines != NULL)
        glDeleteBuffers(1, &data.stroke.EBO_lines);

      glGenBuffers(1, &data.stroke.VBO_position);
      glGenBuffers(1, &data.stroke.VBO_thickness);
      glGenBuffers(1, &data.stroke.VBO_color);
      glGenBuffers(1, &data.stroke.VBO_depth);
      glGenBuffers(1, &data.stroke.EBO_lines);
      data.stroke.drawSize = indices.size();

      glBindVertexArray(data.stroke.VAO);

      glBindBuffer(GL_ARRAY_BUFFER, data.stroke.VBO_position);
      glBufferData(GL_ARRAY_BUFFER, sizeof(position[0]) * numVertices, position.data(), GL_STATIC_DRAW);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
      glEnableVertexAttribArray(0);

      glBindBuffer(GL_ARRAY_BUFFER, data.stroke.VBO_thickness);
      glBufferData(GL_ARRAY_BUFFER, sizeof(thickness[0]) * numVertices, thickness.data(), GL_STATIC_DRAW);
      glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 1 * sizeof(float), (void *)0);
      glEnableVertexAttribArray(1);

      glBindBuffer(GL_ARRAY_BUFFER, data.stroke.VBO_color);
      glBufferData(GL_ARRAY_BUFFER, sizeof(colors[0]) * numVertices, colors.data(), GL_STATIC_DRAW);
      glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
      glEnableVertexAttribArray(2);

      glBindBuffer(GL_ARRAY_BUFFER, data.stroke.VBO_depth);
      glBufferData(GL_ARRAY_BUFFER, sizeof(depths[0]) * numVertices, depths.data(), GL_STATIC_DRAW);
      glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 1 * sizeof(float), (void *)0);
      glEnableVertexAttribArray(3);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.stroke.EBO_lines);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned) * indices.size(), indices.data(), GL_STATIC_DRAW);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
  }

  // filled data
  {
    std::vector<glm::vec3> position;
    std::vector<float> depths;
    std::vector<glm::vec4> colors;
    std::vector<unsigned> indices;
    get_filled_frame_buffer(result.value(), position, colors, depths, indices);
    data.filled.active = !indices.empty();

    if (data.filled.active) {
      const unsigned numVertices = position.size();

      if (data.filled.VBO_position != NULL)
        glDeleteBuffers(1, &data.filled.VBO_position);
      if (data.filled.VBO_depth != NULL)
        glDeleteBuffers(1, &data.filled.VBO_depth);
      if (data.filled.VBO_color != NULL)
        glDeleteBuffers(1, &data.filled.VBO_color);
      if (data.filled.EBO_triangles != NULL)
        glDeleteBuffers(1, &data.filled.EBO_triangles);

      glGenBuffers(1, &data.filled.VBO_position);
      glGenBuffers(1, &data.filled.VBO_depth);
      glGenBuffers(1, &data.filled.VBO_color);
      glGenBuffers(1, &data.filled.EBO_triangles);
      data.filled.drawSize = indices.size();

      glBindVertexArray(data.filled.VAO);

      glBindBuffer(GL_ARRAY_BUFFER, data.filled.VBO_position);
      glBufferData(GL_ARRAY_BUFFER, sizeof(position[0]) * numVertices, position.data(), GL_STATIC_DRAW);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
      glEnableVertexAttribArray(0);

      glBindBuffer(GL_ARRAY_BUFFER, data.filled.VBO_color);
      glBufferData(GL_ARRAY_BUFFER, sizeof(colors[0]) * numVertices, colors.data(), GL_STATIC_DRAW);
      glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
      glEnableVertexAttribArray(1);

      glBindBuffer(GL_ARRAY_BUFFER, data.filled.VBO_depth);
      glBufferData(GL_ARRAY_BUFFER, sizeof(depths[0]) * numVertices, depths.data(), GL_STATIC_DRAW);
      glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 1 * sizeof(float), (void *)0);
      glEnableVertexAttribArray(2);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.filled.EBO_triangles);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned) * indices.size(), indices.data(), GL_STATIC_DRAW);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
  }
  return decorate(new PencilDrawData(data), objPath);
}

glm::mat4 read_maya_matrix(MMatrix m) {
  glm::mat4 mat;
  for (auto row = 0u; row != 4; ++row)
    for (auto col = 0u; col != 4; ++col)
      mat[row][col] = m(row, col);
  return mat;
}

void MPencilDraw::draw(const MHWRender::MDrawContext &context, const MUserData *_data) {
  const PencilDrawData *data = dynamic_cast<const PencilDrawData *>(_data);
  if (!data)
    return;

  glm::mat4 MVP = read_maya_matrix(context.getMatrix(MHWRender::MFrameContext::kWorldViewProjMtx));
  glm::ivec2 vp_origin, vp_size;
  context.getViewportDimensions(vp_origin.x, vp_origin.y, vp_size.x, vp_size.y);
  glm::mat4 projMatrix = read_maya_matrix(context.getMatrix(MHWRender::MFrameContext::kProjectionMtx));
  float len_px = 2.f / std::sqrt(std::min(glm::length(glm::vec3{projMatrix[0]}), glm::length(glm::vec3{projMatrix[1]})));
  float len_sc = std::max(vp_size.x, vp_size.y);
  const float pixelSize = len_px / len_sc;

  //  draw stroke
  if (data->stroke.active) {
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(cPrimitiveRestartIndex);
    glUseProgram(data->stroke.program);
    glUniformMatrix4fv(data->stroke.UNI_MVP, 1, GL_FALSE, glm::value_ptr(MVP));
    glUniformMatrix4fv(data->stroke.UNI_projection, 1, GL_FALSE, glm::value_ptr(projMatrix));
    glUniform1f(data->stroke.UNI_pixelSize, pixelSize);
    glUniform2f(data->stroke.UNI_viewport, vp_size.x, vp_size.y);
    glUniform1ui(data->stroke.UNI_displayStatus, data->displayStatus);
    glBindVertexArray(data->stroke.VAO);
    glDrawElements(GL_LINE_STRIP_ADJACENCY, data->stroke.drawSize, GL_UNSIGNED_INT, nullptr);
  }

  // draw filled
  if (data->filled.active) {
    glUseProgram(data->filled.program);
    glUniformMatrix4fv(data->filled.UNI_MVP, 1, GL_FALSE, glm::value_ptr(MVP));
    glBindVertexArray(data->filled.VAO);
    glDrawElements(GL_TRIANGLES, data->filled.drawSize, GL_UNSIGNED_INT, nullptr);
  }

  glBindVertexArray(0);
  glUseProgram(0);
}
