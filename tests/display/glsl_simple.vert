#version 130

// Uniform inputs
uniform mat4 p3d_ModelViewProjectionMatrix;

// Vertex inputs
in vec4 p3d_Vertex;

void main() {
  gl_Position = p3d_ModelViewProjectionMatrix * p3d_Vertex;
}
