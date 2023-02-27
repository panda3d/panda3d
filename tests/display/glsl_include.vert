#version 330

#extension GL_GOOGLE_include_directive : require

#include "glsl_include_inputs.glsl"

in vec4 p3d_Vertex;

void main() {
  gl_Position = p3d_ModelViewProjectionMatrix * p3d_Vertex;
}
