/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderTerrainMesh.I
 * @author tobspr
 * @date 2016-02-16
 */

/**
 * @brief Sets the heightfield texture
 * @details This sets the heightfield texture. It should be 16bit
 *   single channel, and have a power-of-two resolution greater than 32.
 *   Common sizes are 2048x2048 or 4096x4096.
 *
 *   You should call generate() after setting the heightfield.
 *
 * @param filename Heightfield texture
 */
INLINE void ShaderTerrainMesh::set_heightfield(Texture* heightfield) {
  MutexHolder holder(_lock);
  _heightfield_tex = heightfield;
}

/**
 * @brief Returns the heightfield
 * @details This returns the terrain heightfield, previously set with
 *   set_heightfield()
 *
 * @return Path to the heightfield
 */
INLINE Texture* ShaderTerrainMesh::get_heightfield() const {
  MutexHolder holder(_lock);
  return _heightfield_tex;
}

/**
 * @brief Sets the chunk size
 * @details This sets the chunk size of the terrain. A chunk is basically the
 *   smallest unit in LOD. If the chunk size is too small, the terrain will
 *   perform bad, since there will be way too many chunks. If the chunk size
 *   is too big, you will not get proper LOD, and might also get bad performance.
 *
 *   For terrains of the size 4096x4096 or 8192x8192, a chunk size of 32 seems
 *   to produce good results. For smaller resolutions, you should try out a
 *   size of 16 or even 8 for very small terrains.
 *
 *   The amount of chunks generated for the last level equals to
 *   (heightfield_size / chunk_size) ** 2. The chunk size has to be a power
 *   of two.
 *
 * @param chunk_size Size of the chunks, has to be a power of two
 */
INLINE void ShaderTerrainMesh::set_chunk_size(size_t chunk_size) {
  MutexHolder holder(_lock);
  _chunk_size = chunk_size;
}

/**
 * @brief Returns the chunk size
 * @details This returns the chunk size, previously set with set_chunk_size()
 * @return Chunk size
 */
INLINE size_t ShaderTerrainMesh::get_chunk_size() const {
  MutexHolder holder(_lock);
  return _chunk_size;
}

/**
 * @brief Sets whether to generate patches
 * @details If this option is set to true, GeomPatches will be used instead of
 *   GeomTriangles. This is required when the terrain is used with tesselation
 *   shaders, since patches are required for tesselation, whereas triangles
 *   are required for regular rendering.
 *
 *   If this option is set to true while not using a tesselation shader, the
 *   terrain will not get rendered, or even produce errors. The same applies
 *   when this is option is not set, but the terrain is used with tesselation
 *   shaders.
 *
 * @param generate_patches [description]
 */
INLINE void ShaderTerrainMesh::set_generate_patches(bool generate_patches) {
  MutexHolder holder(_lock);
  _generate_patches = generate_patches;
}

/**
 * @brief Returns whether to generate patches
 * @details This returns whether patches are generated, previously set with
 *   set_generate_patches()
 *
 * @return Whether to generate patches
 */
INLINE bool ShaderTerrainMesh::get_generate_patches() const {
  MutexHolder holder(_lock);
  return _generate_patches;
}


/**
 * @brief Sets the desired triangle width
 * @details This sets the desired width a triangle should have in pixels.
 *   A value of 10.0 for example will make the terrain tesselate everything
 *   in a way that each triangle edge roughly is 10 pixels wide.
 *   Of course this will not always accurately match, however you can use this
 *   setting to control the LOD algorithm of the terrain.
 *
 * @param target_triangle_width Desired triangle width in pixels
 */
INLINE void ShaderTerrainMesh::set_target_triangle_width(PN_stdfloat target_triangle_width) {
  MutexHolder holder(_lock);
  _target_triangle_width = target_triangle_width;
}

/**
 * @brief Returns the target triangle width
 * @details This returns the target triangle width, previously set with
 *   ShaderTerrainMesh::set_target_triangle_width()
 *
 * @return Target triangle width
 */
INLINE PN_stdfloat ShaderTerrainMesh::get_target_triangle_width() const {
  MutexHolder holder(_lock);
  return _target_triangle_width;
}


/**
 * @brief Sets whether to enable terrain updates
 * @details This flag controls whether the terrain should be updated. If this value
 *   is set to false, no updating of the terrain will happen. This can be useful
 *   to debug the culling algorithm used by the terrain.
 *
 * @param update_enabled Whether to update the terrain
 */
INLINE void ShaderTerrainMesh::set_update_enabled(bool update_enabled) {
  MutexHolder holder(_lock);
  _update_enabled = update_enabled;
}

/**
 * @brief Returns whether the terrain is getting updated
 * @details This returns whether the terrain is getting updates, previously set with
 *   set_update_enabled()
 *
 * @return Whether to update the terrain
 */
INLINE bool ShaderTerrainMesh::get_update_enabled() const {
  MutexHolder holder(_lock);
  return _update_enabled;
}

/**
 * @brief Clears all children
 * @details This clears all children on the chunk and sets them to NULL. This will
 *   effectively free all memory consumed by this chunk and its children.
 */
INLINE void ShaderTerrainMesh::Chunk::clear_children() {
  for (size_t i = 0; i < 4; ++i) {
    delete children[i];
    children[i] = nullptr;
  }
}

/**
 * @brief Chunk constructor
 * @details This constructs a new chunk, and sets all children to NULL.
 */
INLINE ShaderTerrainMesh::Chunk::Chunk() {
  for (size_t i = 0; i < 4; ++i)
    children[i] = nullptr;
}

/**
 * @brief Chunk destructor
 * @details This destructs the chunk, freeing all used resources
 */
INLINE ShaderTerrainMesh::Chunk::~Chunk() {
  clear_children();
}

/**
 * @see ShaderTerrainMesh::uv_to_world(LTexCoord)
 */
INLINE LPoint3 ShaderTerrainMesh::uv_to_world(PN_stdfloat u, PN_stdfloat v) const {
  return uv_to_world(LTexCoord(u, v));
}
