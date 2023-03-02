#version 330 core
layout(lines_adjacency) in;
layout(triangle_strip, max_vertices = 10) out;

in VS_OUT {
  float finalThickness;
  vec4 finalColor;
}
gs_in[];

out GEO_OUT {
  vec4 color;
  vec2 coord;
}
geo_out;

uniform vec2 viewport;

/* project 3d point to 2d on screen space */
vec2 toScreenSpace(vec4 vertex) { return vec2(vertex.xy / vertex.w) * viewport; }

/* get zdepth value */
float getZdepth(vec4 point) { return (point.z / point.w); }

/* check equality but with a small tolerance */
bool is_equal(vec4 p1, vec4 p2) {
  float limit = 0.0001;
  float x = abs(p1.x - p2.x);
  float y = abs(p1.y - p2.y);
  float z = abs(p1.z - p2.z);

  if ((x < limit) && (y < limit) && (z < limit)) {
    return true;
  }

  return false;
}

void main(void) {
  float MiterLimit = 0.75;

  /* receive 4 points */
  vec4 P0 = gl_in[0].gl_Position;
  vec4 P1 = gl_in[1].gl_Position;
  vec4 P2 = gl_in[2].gl_Position;
  vec4 P3 = gl_in[3].gl_Position;

  /* get the four vertices passed to the shader */
  vec2 sp0 = toScreenSpace(P0); /* start of previous segment */
  vec2 sp1 = toScreenSpace(P1); /* end of previous segment, start of current segment */
  vec2 sp2 = toScreenSpace(P2); /* end of current segment, start of next segment */
  vec2 sp3 = toScreenSpace(P3); /* end of next segment */

  /* determine the direction of each of the 3 segments (previous, current, next) */
  vec2 v0 = normalize(sp1 - sp0);
  vec2 v1 = normalize(sp2 - sp1);
  vec2 v2 = normalize(sp3 - sp2);

  /* determine the normal of each of the 3 segments (previous, current, next) */
  vec2 n0 = vec2(-v0.y, v0.x);
  vec2 n1 = vec2(-v1.y, v1.x);
  vec2 n2 = vec2(-v2.y, v2.x);

  /* determine miter lines by averaging the normals of the 2 segments */
  vec2 miter_a = normalize(n0 + n1); /* miter at start of current segment */
  vec2 miter_b = normalize(n1 + n2); /* miter at end of current segment */

  /* determine the length of the miter by projecting it onto normal and then inverse it */
  float an1 = dot(miter_a, n1);
  float bn1 = dot(miter_b, n2);
  if (an1 == 0) {
    an1 = 1;
  }
  if (bn1 == 0) {
    bn1 = 1;
  }

  float length_a = gs_in[1].finalThickness / an1;
  float length_b = gs_in[2].finalThickness / bn1;
  if (length_a <= 0.0) {
    length_a = 0.01;
  }
  if (length_b <= 0.0) {
    length_b = 0.01;
  }

  /* prevent excessively long miters at sharp corners */
  if (dot(v0, v1) < -MiterLimit) {
    miter_a = n1;
    length_a = gs_in[1].finalThickness;

    /* close the gap */
    if (dot(v0, n1) > 0) {
      geo_out.coord = vec2(0, 0);
      gl_Position = vec4((sp1 + gs_in[1].finalThickness * n0) / viewport, getZdepth(P1), 1.0);
      geo_out.color = gs_in[1].finalColor;
      EmitVertex();

      geo_out.coord = vec2(0, 0);
      gl_Position = vec4((sp1 + gs_in[1].finalThickness * n1) / viewport, getZdepth(P1), 1.0);
      geo_out.color = gs_in[1].finalColor;
      EmitVertex();

      geo_out.coord = vec2(0, 0.5);
      gl_Position = vec4(sp1 / viewport, getZdepth(P1), 1.0);
      geo_out.color = gs_in[1].finalColor;
      EmitVertex();

      EndPrimitive();
    } else {
      geo_out.coord = vec2(0, 1);
      gl_Position = vec4((sp1 - gs_in[1].finalThickness * n1) / viewport, getZdepth(P1), 1.0);
      geo_out.color = gs_in[1].finalColor;
      EmitVertex();

      geo_out.coord = vec2(0, 1);
      gl_Position = vec4((sp1 - gs_in[1].finalThickness * n0) / viewport, getZdepth(P1), 1.0);
      geo_out.color = gs_in[1].finalColor;
      EmitVertex();

      geo_out.coord = vec2(0, 0.5);
      gl_Position = vec4(sp1 / viewport, getZdepth(P1), 1.0);
      geo_out.color = gs_in[1].finalColor;
      EmitVertex();

      EndPrimitive();
    }
  }

  if (dot(v1, v2) < -MiterLimit) {
    miter_b = n1;
    length_b = gs_in[2].finalThickness;
  }

  /* Generate the start end-cap (alpha < 0 used as end-cap flag). */
  float extend = 1;
  if (is_equal(P0, P2)) {
    geo_out.coord = vec2(1, 0.5);
    geo_out.color = vec4(gs_in[1].finalColor.rgb, gs_in[1].finalColor.a * -1.0);
    vec2 svn1 = normalize(sp1 - sp2) * length_a * 4.0 * extend;
    gl_Position = vec4((sp1 + svn1) / viewport, getZdepth(P1), 1.0);
    EmitVertex();

    geo_out.coord = vec2(0, 0);
    geo_out.color = vec4(gs_in[1].finalColor.rgb, gs_in[1].finalColor.a * -1.0);
    gl_Position = vec4((sp1 - (length_a * 2.0) * miter_a) / viewport, getZdepth(P1), 1.0);
    EmitVertex();

    geo_out.coord = vec2(0, 1);
    geo_out.color = vec4(gs_in[1].finalColor.rgb, gs_in[1].finalColor.a * -1.0);
    gl_Position = vec4((sp1 + (length_a * 2.0) * miter_a) / viewport, getZdepth(P1), 1.0);
    EmitVertex();
  }

  /* generate the triangle strip */
  geo_out.coord = vec2(0, 0);
  gl_Position = vec4((sp1 + length_a * miter_a) / viewport, getZdepth(P1), 1.0);
  geo_out.color = gs_in[1].finalColor;
  EmitVertex();

  geo_out.coord = vec2(0, 1);
  gl_Position = vec4((sp1 - length_a * miter_a) / viewport, getZdepth(P1), 1.0);
  geo_out.color = gs_in[1].finalColor;
  EmitVertex();

  geo_out.coord = vec2(0, 0);
  gl_Position = vec4((sp2 + length_b * miter_b) / viewport, getZdepth(P2), 1.0);
  geo_out.color = gs_in[2].finalColor;
  EmitVertex();

  geo_out.coord = vec2(0, 1);
  gl_Position = vec4((sp2 - length_b * miter_b) / viewport, getZdepth(P2), 1.0);
  geo_out.color = gs_in[2].finalColor;
  EmitVertex();

  /* Generate the end end-cap (alpha < 0 used as end-cap flag). */
  if (is_equal(P1, P3)) {
    geo_out.coord = vec2(0, 1);
    geo_out.color = vec4(gs_in[2].finalColor.rgb, gs_in[2].finalColor.a * -1.0);
    gl_Position = vec4((sp2 + (length_b * 2.0) * miter_b) / viewport, getZdepth(P2), 1.0);
    EmitVertex();

    geo_out.coord = vec2(0, 0);
    geo_out.color = vec4(gs_in[2].finalColor.rgb, gs_in[2].finalColor.a * -1.0);
    gl_Position = vec4((sp2 - (length_b * 2.0) * miter_b) / viewport, getZdepth(P2), 1.0);
    EmitVertex();

    geo_out.coord = vec2(1, 0.5);
    geo_out.color = vec4(gs_in[2].finalColor.rgb, gs_in[2].finalColor.a * -1.0);
    vec2 svn2 = normalize(sp2 - sp1) * length_b * 4.0 * extend;
    gl_Position = vec4((sp2 + svn2) / viewport, getZdepth(P2), 1.0);
    EmitVertex();
  }

  EndPrimitive();
}
