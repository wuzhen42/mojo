#version 330 core
in GEO_OUT {
  vec4 color;
  vec2 coord;
}
geo_in;
out vec4 FragColor;
void main() {
  const vec2 center = vec2(0, 0.5);
  vec4 tColor = geo_in.color;
  if (geo_in.color.a <= 0) {
    tColor.a = tColor.a * -1.0;
    float dist = length(geo_in.coord - center);
    if (dist > 0.25) {
      discard;
    }
  }
  FragColor = tColor;
  // FragColor = vec4(vec3(gl_FragCoord.z), 1.0);
  // FragColor = vec4(vec3(gl_FragCoord), 1.0);
}
