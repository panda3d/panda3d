#version 150

#pragma include <glsl_include_inputs.glsl>

// Vertex inputs
in vec4 p3d_Vertex;

void main() {
  gl_Position = p3d_ModelViewProjectionMatrix * p3d_Vertex;
}
