/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navMeshParams.I
 * @author Maxwell175
 * @date 2022-07-14
 */

/**
 * Initializes a new NavMeshParams object with default parameters.
 */
INLINE NavMeshParams::
NavMeshParams() {
  reset();
}

/**
 * Initializes a new NavMeshParams object as a copy of the given object..
 */
INLINE NavMeshParams::
NavMeshParams(const NavMeshParams &other) {
  cell_size = other.get_cell_size();
  cell_height = other.get_cell_height();
  actor_height = other.get_actor_height();
  actor_radius = other.get_actor_radius();
  actor_max_climb = other.get_actor_max_climb();
  actor_max_slope = other.get_actor_max_slope();
  region_min_size = other.get_region_min_size();
  region_merge_size = other.get_region_merge_size();
  edge_max_len = other.get_edge_max_len();
  edge_max_error = other.get_edge_max_error();
  verts_per_poly = other.get_verts_per_poly();
  detail_sample_dist = other.get_detail_sample_dist();
  detail_sample_max_error = other.get_detail_sample_max_error();
  partition_type = other.get_partition_type();
  tile_size = other.get_tile_size();
  filter_low_hanging_obstacles = other.get_filter_low_hanging_obstacles();
  filter_ledge_spans = other.get_filter_ledge_spans();
  filter_walkable_low_height_spans = other.get_filter_walkable_low_height_spans();
}

/**
 * Sets the height of the actor, which will control how high an opening must be
 * in order for the floor to be considered walkable. Must be greater than or equal to 3.
 */
INLINE void NavMeshParams::
set_actor_height(float height) {
  actor_height = height;
}

/**
 * Returns the height of the actor, which will control how high an opening must be
 * in order for the floor to be considered walkable. Must be greater than or equal to 3.
 */
INLINE float NavMeshParams::
get_actor_height() const {
  return actor_height;
}

/**
 * Sets the radius of the actor, which will be subtracted from the edges of the
 * NavMesh during build.
 */
INLINE void NavMeshParams::
set_actor_radius(float radius) {
  actor_radius = radius;
}

/**
 * Returns the radius of the actor, which will be subtracted from the edges of the
 * NavMesh during build.
 */
INLINE float NavMeshParams::
get_actor_radius() const {
  return actor_radius;
}

/**
 * Sets the maximum height of a ledge that the actor can traverse.
 */
INLINE void NavMeshParams::
set_actor_max_climb(float climb) {
  actor_max_climb = climb;
}

/**
 * Returns the maximum height of a ledge that the actor can traverse.
 */
INLINE float NavMeshParams::
get_actor_max_climb() const {
  return actor_max_climb;
}

/**
 * Sets the maximum slope that is considered walkable. Must be greater than or
 * equal to 0 and less than 90.
 */
INLINE void NavMeshParams::
set_actor_max_slope(float slope) {
  actor_max_slope = slope;
}

/**
 * Returns the maximum slope that is considered walkable. Must be greater than or
 * equal to 0 and less than 90.
 */
INLINE float NavMeshParams::
get_actor_max_slope() const {
  return actor_max_slope;
}

/**
 * Sets the minimum number of cells allowed to form isolated island areas.
 */
INLINE void NavMeshParams::
set_region_min_size(float _region_min_size) {
  region_min_size = _region_min_size;
}

/**
 * Returns the minimum number of cells allowed to form isolated island areas.
 */
INLINE float NavMeshParams::
get_region_min_size() const {
  return region_min_size;
}

/**
 * Sets the size of region under which the region will, if possible,
 * be merged with larger regions.
 */
INLINE void NavMeshParams::
set_region_merge_size(float _region_merge_size) {
  region_merge_size = _region_merge_size;
}

/**
 * Returns the size of region under which the region will, if possible,
 * be merged with larger regions.
 */
INLINE float NavMeshParams::
get_region_merge_size() const {
  return region_merge_size;
}

/**
 * Sets the maximum allowed length for contour edges along the border of the mesh.
 */
INLINE void NavMeshParams::
set_edge_max_len(float max_len) {
  edge_max_len = max_len;
}

/**
 * Returns the maximum allowed length for contour edges along the border of the mesh.
 */
INLINE float NavMeshParams::
get_edge_max_len() const {
  return edge_max_len;
}

/**
 * Sets the maximum distance a simplfied contour's border edges should deviate the
 * original raw contour.
 */
INLINE void NavMeshParams::
set_edge_max_error(float max_error) {
  edge_max_error = max_error;
}

/**
 * Returns the maximum distance a simplfied contour's border edges should deviate the
 * original raw contour.
 */
INLINE float NavMeshParams::
get_edge_max_error() const {
  return edge_max_error;
}

/**
 * Sets the maximum number of vertices allowed for polygons generated during the
 * contour to polygon conversion process. Must be greater than or equal to 3.
 */
INLINE void NavMeshParams::
set_verts_per_poly(float _verts_per_poly) {
  verts_per_poly = _verts_per_poly;
}

/**
 * Returns the maximum number of vertices allowed for polygons generated during the
 * contour to polygon conversion process. Must be greater than or equal to 3.
 */
INLINE float NavMeshParams::
get_verts_per_poly() const {
  return verts_per_poly;
}

/**
 * Sets the horizontal plane cell size to use for fields.
 */
INLINE void NavMeshParams::
set_cell_size(float cs) {
  cell_size = cs;
}

/**
 * Returns the horizontal plane cell size to use for fields.
 */
INLINE float NavMeshParams::
get_cell_size() const {
  return cell_size;
}

/**
 * Sets the vertical axis cell size to use for fields.
 */
INLINE void NavMeshParams::
set_cell_height(float ch) {
  cell_height = ch;
}

/**
 * Returns the vertical axis cell size to use for fields.
 */
INLINE float NavMeshParams::
get_cell_height() const {
  return cell_height;
}

/**
 * Sets the size of a nav mesh tile in cells.
 */
INLINE void NavMeshParams::
set_tile_size(float cs) {
  tile_size = cs;
}

/**
 * Returns the size of a nav mesh tile in cells.
 */
INLINE float NavMeshParams::
get_tile_size() const {
  return tile_size;
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
INLINE void NavMeshParams::
set_partition_type(NavMeshParams::PartitionType partition) {
  partition_type = partition;
}

/**
 * Returns the partitioning method to use when building the mesh.
 */
INLINE NavMeshParams::PartitionType NavMeshParams::
get_partition_type() const {
  return partition_type;
}

/**
 * Returns the maximum number of tiles that are possible.
 */
INLINE int NavMeshParams::
get_max_tiles() const {
  // Max tiles and max polys affect how the tile IDs are calculated.
  // There are 22 bits available for identifying a tile and a polygon.
  // We will default to the max recommended number of tile bits.
  int tileBits = 14;
  int polyBits = 22 - tileBits;
  return 1 << tileBits;
}

/**
 * Returns the maximum number of polygons that can be in a single tile.
 */
INLINE int NavMeshParams::
get_max_polys_per_tile() const {
  // Max tiles and max polys affect how the tile IDs are calculated.
  // There are 22 bits available for identifying a tile and a polygon.
  // We will default to the max recommended number of tile bits.
  int tileBits = 14;
  int polyBits = 22 - tileBits;
  return 1 << polyBits;
}

/**
 * Sets the sampling distance to use when generating the detail mesh.
 */
INLINE void NavMeshParams::
set_detail_sample_dist(float dsd) {
  detail_sample_dist = dsd;
}

/**
 * Returns the sampling distance to use when generating the detail mesh.
 */
INLINE float NavMeshParams::
get_detail_sample_dist() const {
  return detail_sample_dist;
}

/**
 * Sets the maximum distance the detail mesh surface should deviate from heightfield
 * data. (For height detail only.) [Limit: >=0]
 */
INLINE void NavMeshParams::
set_detail_sample_max_error(float dsme) {
  detail_sample_max_error = dsme;
}

/**
 * Returns the maximum distance the detail mesh surface should deviate from heightfield
 * data. (For height detail only.) [Limit: >=0]
 */
INLINE float NavMeshParams::
get_detail_sample_max_error() const {
  return detail_sample_max_error;
}

/**
 * Returns the original tile origin point that will be used to count tiles.
 */
INLINE LPoint3 NavMeshParams::
get_orig_bound_min() const {
  return LPoint3(orig_bound_min[0], orig_bound_min[1], orig_bound_min[2]);
}

/**
 * Returns the raw tile size.
 */
INLINE float NavMeshParams::
get_tile_cell_size() const {
  return tile_size * cell_size;
}

/**
 * Sets whether to mark non-walkable spans as walkable if their maximum is within actor_max_climb of a walkable neighbor.
 */
INLINE void NavMeshParams::
set_filter_low_hanging_obstacles(bool flho) {
  filter_low_hanging_obstacles = flho;
}

/**
 * Returns whether to mark non-walkable spans as walkable if their maximum is within actor_max_climb of a walkable neighbor.
 */
INLINE bool NavMeshParams::
get_filter_low_hanging_obstacles() const {
  return filter_low_hanging_obstacles;
}

/**
 * Sets whether to mark spans that are ledges as not-walkable.
 */
INLINE void NavMeshParams::
set_filter_ledge_spans(bool fls) {
  filter_ledge_spans = fls;
}

/**
 * Returns whether to mark spans that are ledges as not-walkable.
 */
INLINE bool NavMeshParams::
get_filter_ledge_spans() const {
  return filter_ledge_spans;
}

/**
 * Sets whether to mark walkable spans as not walkable if the clearance above the span is less than the specified height.
 */
INLINE void NavMeshParams::
set_filter_walkable_low_height_spans(bool fwlhs) {
  filter_walkable_low_height_spans = fwlhs;
}

/**
 * Returns whether to mark walkable spans as not walkable if the clearance above the span is less than the specified height.
 */
INLINE bool NavMeshParams::
get_filter_walkable_low_height_spans() const {
  return filter_walkable_low_height_spans;
}

INLINE void NavMeshParams::
reset() {
  cell_size = 0.3f;
  cell_height = 0.2f;
  actor_height = 2.0f;
  actor_radius = 0.6f;
  actor_max_climb = 0.9f;
  actor_max_slope = 45.0f;
  region_min_size = 8;
  region_merge_size = 20;
  edge_max_len = 12.0f;
  edge_max_error = 1.3f;
  verts_per_poly = 6.0f;
  detail_sample_dist = 6.0f;
  detail_sample_max_error = 1.0f;
  partition_type = NavMeshParams::SAMPLE_PARTITION_LAYERS;
  tile_size = 32;
  filter_low_hanging_obstacles = true;
  filter_ledge_spans = true;
  filter_walkable_low_height_spans = true;
}

INLINE bool NavMeshParams::
operator==(const NavMeshParams &other) const {
  return std::make_tuple(
      cell_size,
      cell_height,
      actor_height,
      actor_radius,
      actor_max_climb,
      actor_max_slope,
      region_min_size,
      region_merge_size,
      edge_max_len,
      edge_max_error,
      verts_per_poly,
      detail_sample_dist,
      detail_sample_max_error ,
      partition_type,
      tile_size,
      filter_low_hanging_obstacles,
      filter_ledge_spans,
      filter_walkable_low_height_spans
      ) ==
         std::make_tuple(
             other.get_cell_size(),
             other.get_cell_height(),
             other.get_actor_height(),
             other.get_actor_radius(),
             other.get_actor_max_climb(),
             other.get_actor_max_slope(),
             other.get_region_min_size(),
             other.get_region_merge_size(),
             other.get_edge_max_len(),
             other.get_edge_max_error(),
             other.get_verts_per_poly(),
             other.get_detail_sample_dist(),
             other.get_detail_sample_max_error(),
             other.get_partition_type(),
             other.get_tile_size(),
             other.get_filter_low_hanging_obstacles(),
             other.get_filter_ledge_spans(),
             other.get_filter_walkable_low_height_spans()
             );
}
