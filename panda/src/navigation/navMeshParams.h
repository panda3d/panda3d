/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navMeshParams.h
 * @author Maxwell175
 * @date 2022-07-14
 */


#ifndef NAVMESHPARAMS_H
#define NAVMESHPARAMS_H

#include "pandabase.h"
#include "pandaSystem.h"
#include "luse.h"

/**
 * NavMeshParams class stores all the parameters of a navigation mesh.
 */
class NavMeshParams {
PUBLISHED:
  enum PartitionType {
    SAMPLE_PARTITION_WATERSHED,
    SAMPLE_PARTITION_MONOTONE,
    SAMPLE_PARTITION_LAYERS,
  };

  INLINE NavMeshParams();
  INLINE NavMeshParams(const NavMeshParams &other);

  INLINE float get_actor_height() const;
  INLINE float get_actor_radius() const;
  INLINE float get_actor_max_climb() const;
  INLINE float get_actor_max_slope() const;
  INLINE float get_region_min_size() const;
  INLINE float get_region_merge_size() const;
  INLINE float get_edge_max_len() const;
  INLINE float get_edge_max_error() const;
  INLINE int get_verts_per_poly() const;
  INLINE float get_cell_size() const;
  INLINE float get_cell_height() const;
  INLINE float get_tile_size() const;
  INLINE NavMeshParams::PartitionType get_partition_type() const;
  INLINE int get_max_tiles() const;
  INLINE int get_max_polys_per_tile() const;
  INLINE float get_detail_sample_dist() const;
  INLINE float get_detail_sample_max_error() const;
  INLINE LPoint3 get_orig_bound_min() const;
  INLINE float get_tile_cell_size() const;
  INLINE bool get_filter_low_hanging_obstacles() const;
  INLINE bool get_filter_ledge_spans() const;
  INLINE bool get_filter_walkable_low_height_spans() const;

  INLINE void set_actor_height(float height);
  INLINE void set_actor_radius(float radius);
  INLINE void set_actor_max_climb(float climb);
  INLINE void set_actor_max_slope(float slope);
  INLINE void set_region_min_size(float region_min_size);
  INLINE void set_region_merge_size(float region_merge_size);
  INLINE void set_edge_max_len(float max_len);
  INLINE void set_edge_max_error(float max_error);
  INLINE void set_verts_per_poly(int verts_per_poly);
  INLINE void set_cell_size(float cs);
  INLINE void set_cell_height(float ch);
  INLINE void set_tile_size(float cs);
  INLINE void set_partition_type(NavMeshParams::PartitionType partition);
  INLINE void set_detail_sample_dist(float detail_sample_dist);
  INLINE void set_detail_sample_max_error(float detail_sample_max_error);
  INLINE void set_filter_low_hanging_obstacles(bool filter_low_hanging_obstacles);
  INLINE void set_filter_ledge_spans(bool filter_ledge_spans);
  INLINE void set_filter_walkable_low_height_spans(bool filter_walkable_low_height_spans);

  MAKE_PROPERTY(actor_radius, get_actor_radius, set_actor_radius);
  MAKE_PROPERTY(actor_height, get_actor_height, set_actor_height);
  MAKE_PROPERTY(actor_max_climb, get_actor_max_climb, set_actor_max_climb);
  MAKE_PROPERTY(actor_max_slope, get_actor_max_slope, set_actor_max_slope);
  MAKE_PROPERTY(region_min_size, get_region_min_size, set_region_min_size);
  MAKE_PROPERTY(region_merge_size, get_region_merge_size, set_region_merge_size);
  MAKE_PROPERTY(edge_max_len, get_edge_max_len, set_edge_max_len);
  MAKE_PROPERTY(edge_max_error, get_edge_max_error, set_edge_max_error);
  MAKE_PROPERTY(verts_per_poly, get_verts_per_poly, set_verts_per_poly);
  MAKE_PROPERTY(cell_size, get_cell_size, set_cell_size);
  MAKE_PROPERTY(cell_height, get_cell_height, set_cell_height);
  MAKE_PROPERTY(tile_size, get_tile_size, set_tile_size);
  MAKE_PROPERTY(partition_type, get_partition_type, set_partition_type);
  MAKE_PROPERTY(max_tiles, get_max_tiles);
  MAKE_PROPERTY(max_polys_per_tile, get_max_polys_per_tile);
  MAKE_PROPERTY(detail_sample_dist, get_detail_sample_dist, set_detail_sample_dist);
  MAKE_PROPERTY(detail_sample_max_error, get_detail_sample_max_error, set_detail_sample_max_error);
  MAKE_PROPERTY(orig_bound_min, get_orig_bound_min);
  MAKE_PROPERTY(tile_cell_size, get_tile_cell_size);
  MAKE_PROPERTY(filter_low_hanging_obstacles, get_filter_low_hanging_obstacles, set_filter_low_hanging_obstacles);
  MAKE_PROPERTY(filter_ledge_spans, get_filter_ledge_spans, set_filter_ledge_spans);
  MAKE_PROPERTY(filter_walkable_low_height_spans, get_filter_walkable_low_height_spans, set_filter_walkable_low_height_spans);

  INLINE void reset();

  INLINE bool operator==(const NavMeshParams &other) const;

protected:
  float actor_height;
  float actor_radius;
  float actor_max_climb;
  float actor_max_slope;
  float region_min_size;
  float region_merge_size;
  float edge_max_len;
  float edge_max_error;
  int verts_per_poly;
  float cell_size;
  float cell_height;
  float tile_size;
  PartitionType partition_type;
  float detail_sample_dist;
  float detail_sample_max_error;
  float orig_bound_min[3];
  float tile_cell_size;
  bool filter_low_hanging_obstacles;
  bool filter_ledge_spans;
  bool filter_walkable_low_height_spans;
};

#include "navMeshParams.I"

#endif // NAVMESHPARAMS_H
