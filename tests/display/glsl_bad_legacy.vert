#version 120

// Uniform inputs
uniform mat4 p3d_ModelViewProjectionMatrix;

// Vertex inputs
attribute vec4 p3d_Vertex;

void main() {
  gl_Position = p3d_ModelViewProjectionMatrix * p3d_Vertex;
  does_not_exist = p3d_Vertex;
}
