/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sceneGraphAnalyzer.I
 * @author drose
 * @date 2006-10-15
 */

/**
 * Specifies the mode in which LODNodes are analyzed.
 */
INLINE void SceneGraphAnalyzer::
set_lod_mode(LodMode lod_mode) {
  _lod_mode = lod_mode;
}

/**
 * Returns the mode in which LODNodes are analyzed.
 */
INLINE  SceneGraphAnalyzer::LodMode SceneGraphAnalyzer::
get_lod_mode(LodMode lod_mode) const {
  return _lod_mode;
}

/**
 *
 */
int SceneGraphAnalyzer::
get_num_nodes() const {
  return _num_nodes;
}

/**
 *
 */
int SceneGraphAnalyzer::
get_num_instances() const {
  return _num_instances;
}

/**
 *
 */
int SceneGraphAnalyzer::
get_num_transforms() const {
  return _num_transforms;
}

/**
 *
 */
int SceneGraphAnalyzer::
get_num_nodes_with_attribs() const {
  return _num_nodes_with_attribs;
}

/**
 *
 */
int SceneGraphAnalyzer::
get_num_lod_nodes() const {
  return _num_lod_nodes;
}

/**
 *
 */
int SceneGraphAnalyzer::
get_num_geom_nodes() const {
  return _num_geom_nodes;
}

/**
 *
 */
int SceneGraphAnalyzer::
get_num_geoms() const {
  return _num_geoms;
}

/**
 *
 */
int SceneGraphAnalyzer::
get_num_geom_vertex_datas() const {
  return _num_geom_vertex_datas;
}

/**
 *
 */
int SceneGraphAnalyzer::
get_num_geom_vertex_formats() const {
  return _num_geom_vertex_formats;
}

/**
 *
 */
size_t SceneGraphAnalyzer::
get_vertex_data_size() const {
  return _vertex_data_size;
}

/**
 *
 */
int SceneGraphAnalyzer::
get_num_vertices() const {
  return _num_vertices;
}

/**
 *
 */
int SceneGraphAnalyzer::
get_num_normals() const {
  return _num_normals;
}

/**
 *
 */
int SceneGraphAnalyzer::
get_num_colors() const {
  return _num_colors;
}

/**
 *
 */
int SceneGraphAnalyzer::
get_num_texcoords() const {
  return _num_texcoords;
}

/**
 *
 */
int SceneGraphAnalyzer::
get_num_tris() const {
  return _num_tris;
}

/**
 *
 */
int SceneGraphAnalyzer::
get_num_lines() const {
  return _num_lines;
}

/**
 *
 */
int SceneGraphAnalyzer::
get_num_points() const {
  return _num_points;
}

/**
 *
 */
int SceneGraphAnalyzer::
get_num_patches() const {
  return _num_patches;
}


/**
 *
 */
int SceneGraphAnalyzer::
get_num_individual_tris() const {
  return _num_individual_tris;
}

/**
 *
 */
int SceneGraphAnalyzer::
get_num_tristrips() const {
  return _num_tristrips;
}

/**
 *
 */
int SceneGraphAnalyzer::
get_num_triangles_in_strips() const {
  return _num_triangles_in_strips;
}

/**
 *
 */
int SceneGraphAnalyzer::
get_num_trifans() const {
  return _num_trifans;
}

/**
 *
 */
int SceneGraphAnalyzer::
get_num_triangles_in_fans() const {
  return _num_triangles_in_fans;
}

/**
 *
 */
int SceneGraphAnalyzer::
get_num_vertices_in_patches() const {
  return _num_vertices_in_patches;
}


/**
 *
 */
size_t SceneGraphAnalyzer::
get_texture_bytes() const {
  return _texture_bytes;
}


/**
 *
 */
int SceneGraphAnalyzer::
get_num_long_normals() const {
  return _num_long_normals;
}

/**
 *
 */
int SceneGraphAnalyzer::
get_num_short_normals() const {
  return _num_short_normals;
}

/**
 *
 */
PN_stdfloat SceneGraphAnalyzer::
get_total_normal_length() const {
  return _total_normal_length;
}
