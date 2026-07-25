/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navMeshSettings.I
 * @author Ashwani / 
 * @date 2024
 */

/**
 * Sets the cell size in world units.
 */
INLINE void NavMeshSettings::
set_cell_size(PN_stdfloat cs) {
  _cell_size = cs;
}

/**
 * Returns the cell size in world units.
 */
INLINE PN_stdfloat NavMeshSettings::
get_cell_size() const {
  return _cell_size;
}

/**
 * Sets the cell height in world units.
 */
INLINE void NavMeshSettings::
set_cell_height(PN_stdfloat ch) {
  _cell_height = ch;
}

/**
 * Returns the cell height in world units.
 */
INLINE PN_stdfloat NavMeshSettings::
get_cell_height() const {
  return _cell_height;
}

/**
 * Sets the agent height in world units.
 */
INLINE void NavMeshSettings::
set_agent_height(PN_stdfloat height) {
  _agent_height = height;
}

/**
 * Returns the agent height in world units.
 */
INLINE PN_stdfloat NavMeshSettings::
get_agent_height() const {
  return _agent_height;
}

/**
 * Sets the agent radius in world units.
 */
INLINE void NavMeshSettings::
set_agent_radius(PN_stdfloat radius) {
  _agent_radius = radius;
}

/**
 * Returns the agent radius in world units.
 */
INLINE PN_stdfloat NavMeshSettings::
get_agent_radius() const {
  return _agent_radius;
}

/**
 * Sets the agent maximum climb height in world units.
 */
INLINE void NavMeshSettings::
set_agent_max_climb(PN_stdfloat climb) {
  _agent_max_climb = climb;
}

/**
 * Returns the agent maximum climb height in world units.
 */
INLINE PN_stdfloat NavMeshSettings::
get_agent_max_climb() const {
  return _agent_max_climb;
}

/**
 * Sets the agent maximum slope in degrees.
 */
INLINE void NavMeshSettings::
set_agent_max_slope(PN_stdfloat slope) {
  _agent_max_slope = slope;
}

/**
 * Returns the agent maximum slope in degrees.
 */
INLINE PN_stdfloat NavMeshSettings::
get_agent_max_slope() const {
  return _agent_max_slope;
}

/**
 * Sets the region minimum size.
 */
INLINE void NavMeshSettings::
set_region_min_size(PN_stdfloat size) {
  _region_min_size = size;
}

/**
 * Returns the region minimum size.
 */
INLINE PN_stdfloat NavMeshSettings::
get_region_min_size() const {
  return _region_min_size;
}

/**
 * Sets the region merge size.
 */
INLINE void NavMeshSettings::
set_region_merge_size(PN_stdfloat size) {
  _region_merge_size = size;
}

/**
 * Returns the region merge size.
 */
INLINE PN_stdfloat NavMeshSettings::
get_region_merge_size() const {
  return _region_merge_size;
}

/**
 * Sets the edge maximum length.
 */
INLINE void NavMeshSettings::
set_edge_max_len(PN_stdfloat len) {
  _edge_max_len = len;
}

/**
 * Returns the edge maximum length.
 */
INLINE PN_stdfloat NavMeshSettings::
get_edge_max_len() const {
  return _edge_max_len;
}

/**
 * Sets the edge maximum error.
 */
INLINE void NavMeshSettings::
set_edge_max_error(PN_stdfloat error) {
  _edge_max_error = error;
}

/**
 * Returns the edge maximum error.
 */
INLINE PN_stdfloat NavMeshSettings::
get_edge_max_error() const {
  return _edge_max_error;
}

/**
 * Sets the maximum number of vertices per polygon.
 */
INLINE void NavMeshSettings::
set_verts_per_poly(int verts) {
  _verts_per_poly = verts;
}

/**
 * Returns the maximum number of vertices per polygon.
 */
INLINE int NavMeshSettings::
get_verts_per_poly() const {
  return _verts_per_poly;
}

/**
 * Sets the detail sample distance.
 */
INLINE void NavMeshSettings::
set_detail_sample_dist(PN_stdfloat dist) {
  _detail_sample_dist = dist;
}

/**
 * Returns the detail sample distance.
 */
INLINE PN_stdfloat NavMeshSettings::
get_detail_sample_dist() const {
  return _detail_sample_dist;
}

/**
 * Sets the detail sample maximum error.
 */
INLINE void NavMeshSettings::
set_detail_sample_max_error(PN_stdfloat error) {
  _detail_sample_max_error = error;
}

/**
 * Returns the detail sample maximum error.
 */
INLINE PN_stdfloat NavMeshSettings::
get_detail_sample_max_error() const {
  return _detail_sample_max_error;
}

