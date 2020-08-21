/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file preparedGraphicsObjects.I
 * @author drose
 * @date 2004-02-23
 */

/**
 * Returns the name of the PreparedGraphicsObjects structure.  This is an
 * arbitrary name that serves mainly to uniquify the context for PStats
 * reporting.
 */
INLINE const std::string &PreparedGraphicsObjects::
get_name() const {
  return _name;
}

/**
 * Returns the artificial cap on graphics memory that will be imposed on this
 * GSG.  See set_graphics_memory_limit().
 */
INLINE size_t PreparedGraphicsObjects::
get_graphics_memory_limit() const {
  return _graphics_memory_lru.get_max_size();
}

/**
 * Releases all prepared objects of all kinds at once.
 */
INLINE void PreparedGraphicsObjects::
release_all() {
  release_all_textures();
  release_all_samplers();
  release_all_geoms();
  release_all_shaders();
  release_all_vertex_buffers();
  release_all_index_buffers();

  _texture_residency.set_levels();
  _vbuffer_residency.set_levels();
  _ibuffer_residency.set_levels();
  _sbuffer_residency.set_levels();
}

/**
 * Returns the number of objects of any kind that have been enqueued to be
 * prepared on this GSG.
 */
INLINE int PreparedGraphicsObjects::
get_num_queued() const {
  return (get_num_queued_textures() +
          get_num_queued_samplers() +
          get_num_queued_geoms() +
          get_num_queued_shaders() +
          get_num_queued_vertex_buffers() +
          get_num_queued_index_buffers() +
          get_num_queued_shader_buffers());
}

/**
 * Returns the number of objects of any kind that have already been prepared
 * on this GSG.
 */
INLINE int PreparedGraphicsObjects::
get_num_prepared() const {
  return (get_num_prepared_textures() +
          get_num_prepared_samplers() +
          get_num_prepared_geoms() +
          get_num_prepared_shaders() +
          get_num_prepared_vertex_buffers() +
          get_num_prepared_index_buffers() +
          get_num_prepared_shader_buffers());
}

/**
 *
 */
INLINE bool PreparedGraphicsObjects::BufferCacheKey::
operator < (const PreparedGraphicsObjects::BufferCacheKey &other) const {
  if (_data_size_bytes != other._data_size_bytes) {
    return _data_size_bytes < other._data_size_bytes;
  }
  return (int)_usage_hint < (int)other._usage_hint;
}

/**
 *
 */
INLINE bool PreparedGraphicsObjects::BufferCacheKey::
operator == (const PreparedGraphicsObjects::BufferCacheKey &other) const {
  return (_data_size_bytes == other._data_size_bytes &&
          _usage_hint == other._usage_hint);
}

/**
 *
 */
INLINE bool PreparedGraphicsObjects::BufferCacheKey::
operator != (const PreparedGraphicsObjects::BufferCacheKey &other) const {
  return !operator == (other);
}
