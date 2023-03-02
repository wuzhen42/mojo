#version 330 core
layout(location = 0) in vec3 point;
layout(location = 1) in vec4 color;
layout(location = 2) in float depth;
uniform mat4 MVP;

out vec4 finalColor;

void main() {
  vec4 finalPosition = MVP * vec4(point, 1.0);
  finalPosition.z -= float(depth) * 0.001 * finalPosition.w;
  gl_Position = finalPosition;
  finalColor = color;
}