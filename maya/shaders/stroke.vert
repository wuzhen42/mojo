#version 330 core
layout(location = 0) in vec3 point;
layout(location = 1) in float thickness;
layout(location = 2) in vec4 color;
layout(location = 3) in float depth;
uniform mat4 MVP;
uniform mat4 projection;
uniform float pixelsize;
uniform vec2 viewport;
uniform uint displayStatus;

float defaultPixelSize = pixelsize * 1000.0;

const uint cDisplayLead = 8u;
const uint cDisplayActive = 0u;

out VS_OUT {
  float finalThickness;
  vec4 finalColor;
}
vs_out;

void main() {
  vec4 finalPosition = MVP * vec4(point, 1.0);
  finalPosition.z -= float(depth) * 0.001 * finalPosition.w;
  gl_Position = finalPosition;

  float size = (projection[3][3] == 0) ? (thickness / (gl_Position.z * defaultPixelSize)) : (thickness / defaultPixelSize);
  vs_out.finalThickness = max(size, 1.0);
  if (cDisplayLead == displayStatus)
    vs_out.finalColor = vec4(0.26, 1.0, 0.64, 1.0);
  else if (cDisplayActive == displayStatus)
    vs_out.finalColor = vec4(1.0, 1.0, 1.0, 1.0);
  else
    vs_out.finalColor = color;
}
