/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navMeshBuilder.I
 * @author Maxwell175
 * @date 2022-02-21
 */


/**
 * Sets the height of the actor, which will control how high an opening must be
 * in order for the floor to be considered walkable. Must be greater than or equal to 3.
 */
INLINE void NavMeshBuilder::
set_actor_height(float height) {
  _agent_height = height;
}

/**
 * Returns the height of the actor, which will control how high an opening must be
 * in order for the floor to be considered walkable. Must be greater than or equal to 3.
 */
INLINE float NavMeshBuilder::
get_actor_height() const {
  return _agent_height;
}

/**
 * Sets the radius of the actor, which will be subtracted from the edges of the
 * NavMesh during build.
 */
INLINE void NavMeshBuilder::
set_actor_radius(float radius) {
  _agent_radius = radius;
}

/**
 * Returns the radius of the actor, which will be subtracted from the edges of the
 * NavMesh during build.
 */
INLINE float NavMeshBuilder::
get_actor_radius() const {
  return _agent_radius;
}

/**
 * Sets the maximum height of a ledge that the actor can traverse.
 */
INLINE void NavMeshBuilder::
set_actor_max_climb(float climb) {
  _agent_max_climb = climb;
}

/**
 * Returns the maximum height of a ledge that the actor can traverse.
 */
INLINE float NavMeshBuilder::
get_actor_max_climb() const {
  return _agent_max_climb;
}

/**
 * Sets the maximum slope that is considered walkable. Must be greater than or
 * equal to 0 and less than 90.
 */
INLINE void NavMeshBuilder::
set_actor_max_slope(float slope) {
  _agent_max_slope = slope;
}

/**
 * Returns the maximum slope that is considered walkable. Must be greater than or
 * equal to 0 and less than 90.
 */
INLINE float NavMeshBuilder::
get_actor_max_slope() const {
  return _agent_max_slope;
}

/**
 * Sets the minimum number of cells allowed to form isolated island areas.
 */
INLINE void NavMeshBuilder::
set_region_min_size(float region_min_size) {
  _region_min_size = region_min_size;
}

/**
 * Returns the minimum number of cells allowed to form isolated island areas.
 */
INLINE float NavMeshBuilder::
get_region_min_size() const {
  return _region_min_size;
}

/**
 * Sets the size of region under which the region will, if possible,
 * be merged with larger regions.
 */
INLINE void NavMeshBuilder::
set_region_merge_size(float region_merge_size) {
  _region_merge_size = region_merge_size;
}

/**
 * Returns the size of region under which the region will, if possible,
 * be merged with larger regions.
 */
INLINE float NavMeshBuilder::
get_region_merge_size() const {
  return _region_merge_size;
}

/**
 * Sets the maximum allowed length for contour edges along the border of the mesh.
 */
INLINE void NavMeshBuilder::
set_edge_max_len(float max_len) {
  _edge_max_len = max_len;
}

/**
 * Returns the maximum allowed length for contour edges along the border of the mesh.
 */
INLINE float NavMeshBuilder::
get_edge_max_len() const {
return _edge_max_len;
}

/**
 * Sets the maximum distance a simplfied contour's border edges should deviate the
 * original raw contour.
 */
INLINE void NavMeshBuilder::
set_edge_max_error(float max_error) {
  _edge_max_error = max_error;
}

/**
 * Returns the maximum distance a simplfied contour's border edges should deviate the
 * original raw contour.
 */
INLINE float NavMeshBuilder::
get_edge_max_error() const {
  return _edge_max_error;
}

/**
 * Sets the maximum number of vertices allowed for polygons generated during the
 * contour to polygon conversion process. Must be greater than or equal to 3.
 */
INLINE void NavMeshBuilder::
set_verts_per_poly(float verts_per_poly) {
  _verts_per_poly = verts_per_poly;
}

/**
 * Returns the maximum number of vertices allowed for polygons generated during the
 * contour to polygon conversion process. Must be greater than or equal to 3.
 */
INLINE float NavMeshBuilder::
get_verts_per_poly() const {
  return _verts_per_poly;
}

/**
 * Sets the horizontal plane cell size to use for fields.
 */
INLINE void NavMeshBuilder::
set_cell_size(float cs) {
  _cell_size = cs;
}

/**
 * Returns the horizontal plane cell size to use for fields.
 */
INLINE float NavMeshBuilder::
get_cell_size() const {
  return _cell_size;
}

/**
 * Sets the vertical axis cell size to use for fields.
 */
INLINE void NavMeshBuilder::
set_cell_height(float ch) {
  _cell_height = ch;
}

/**
 * Returns the vertical axis cell size to use for fields.
 */
INLINE float NavMeshBuilder::
get_cell_height() const {
  return _cell_height;
}

/**
 * Sets the partitioning method to use when building the mesh.
 *
 * There are 3 partitioning methods, each with some pros and cons:
 * 1) Watershed partitioning
 *   - the classic Recast partitioning
 *   - creates the nicest tessellation
 *   - usually slowest
 *   - partitions the heightfield into nice regions without holes or overlaps
 *   - the are some corner cases where this method creates produces holes and overlaps
 *      - holes may appear when a small obstacles is close to large open area (triangulation can handle this)
 *      - overlaps may occur if you have narrow spiral corridors (i.e stairs), this make triangulation to fail
 *   * Generally the best choice if you precompute the navmesh, use this if you have large open areas
 * 2) Monotone partitioning
 *   - fastest
 *   - partitions the heightfield into regions without holes and overlaps (guaranteed)
 *   - creates long thin polygons, which sometimes causes paths with detours
 *   * Use this if you want fast navmesh generation
 * 3) Layer partitioning
 *   - quite fast
 *   - partitions the heightfield into non-overlapping regions
 *   - relies on the triangulation code to cope with holes (thus slower than monotone partitioning)
 *   - produces better triangles than monotone partitioning
 *   - does not have the corner cases of watershed partitioning
 *   - can be slow and create a bit ugly tessellation (still better than monotone)
 *     if you have large open areas with small obstacles (not a problem if you use tiles)
 *   * Good choice to use for tiled navmesh with medium and small sized tiles
 */
INLINE void NavMeshBuilder::
set_partition_type(NavMeshBuilder::PartitionType partition) {
  _partition_type = partition;
}

/**
 * Returns the partitioning method to use when building the mesh.
 */
INLINE NavMeshBuilder::PartitionType NavMeshBuilder::
get_partition_type() const {
  return _partition_type;
}

/**
 * Returns the number of vertices that have been loaded from the source geometry.
 */
INLINE size_t NavMeshBuilder::
get_vert_count() const {
  return _verts.size() / 3;
}

/**
 * Returns the number of tris that have been loaded from the source geometry.
 */
INLINE size_t NavMeshBuilder::
get_tri_count() const {
  return _tris.size() / 3;
}

/**
 * Returns the number of vertices that made it into the NavMesh.
 */
INLINE int NavMeshBuilder::
get_pmesh_vert_count() const {
  return _pmesh->nverts;
}

/**
 * Returns the number of polygons that comprise the generated NavMesh.
 */
INLINE int NavMeshBuilder::
get_pmesh_poly_count() const {
  return _pmesh->npolys;
}

/**
 * Returns the number of polygons that comprise the generated NavMesh.
 */
INLINE float NavMeshBuilder::
get_build_time_ms() const {
  return _total_build_time_ms;
}
