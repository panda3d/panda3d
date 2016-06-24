#version 150

// This is the terrain fragment shader. There is a lot of code in here
// which is not necessary to render the terrain, but included for convenience -
// Like generating normals from the heightmap or a simple fog effect.

// Most of the time you want to adjust this shader to get your terrain the look
// you want. The vertex shader most likely will stay the same.

in vec2 terrain_uv;
in vec3 vtx_pos;
out vec4 color;

uniform struct {
  sampler2D data_texture;
  sampler2D heightfield;
  int view_index;
  int terrain_size;
  int chunk_size;
} ShaderTerrainMesh;

uniform sampler2D p3d_Texture0;
uniform vec3 wspos_camera;

// Compute normal from the heightmap, this assumes the terrain is facing z-up
vec3 get_terrain_normal() {
  const float terrain_height = 50.0;
  vec3 pixel_size = vec3(1.0, -1.0, 0) / textureSize(ShaderTerrainMesh.heightfield, 0).xxx;
  float u0 = texture(ShaderTerrainMesh.heightfield, terrain_uv + pixel_size.yz).x * terrain_height;
  float u1 = texture(ShaderTerrainMesh.heightfield, terrain_uv + pixel_size.xz).x * terrain_height;
  float v0 = texture(ShaderTerrainMesh.heightfield, terrain_uv + pixel_size.zy).x * terrain_height;
  float v1 = texture(ShaderTerrainMesh.heightfield, terrain_uv + pixel_size.zx).x * terrain_height;
  vec3 tangent = normalize(vec3(1.0, 0, u1 - u0));
  vec3 binormal = normalize(vec3(0, 1.0, v1 - v0));
  return normalize(cross(tangent, binormal));
}



void main() {
  vec3 diffuse = texture(p3d_Texture0, terrain_uv * 16.0).xyz;
  vec3 normal = get_terrain_normal();

  // Add some fake lighting - you usually want to use your own lighting code here
  vec3 fake_sun = normalize(vec3(0.7, 0.2, 0.6));
  vec3 shading = max(0.0, dot(normal, fake_sun)) * diffuse;
  shading += vec3(0.07, 0.07, 0.1);


  // Fake fog
  float dist = distance(vtx_pos, wspos_camera);
  float fog_factor = smoothstep(0, 1, dist / 1000.0);
  shading = mix(shading, vec3(0.7, 0.7, 0.8), fog_factor);

  color = vec4(shading, 1.0);
}
