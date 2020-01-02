#version 130

#pragma include "glsl_include_inputs.vert"

void main() {
  gl_Position = p3d_ModelViewProjectionMatrix * p3d_Vertex;
}
