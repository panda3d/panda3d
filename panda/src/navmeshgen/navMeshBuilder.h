/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navMeshBuilder.h
 * @author ashwini
 * @date 2020-06-21
 */


#ifndef NAVMESHBUILDER_H
#define NAVMESHBUILDER_H

#include "recastnavigation/Recast.h"
#include <string>
#include <unordered_map>
#include <tuple>
#include "geom.h"
#include "geomNode.h"
#include "nodePath.h"
#include "pandaSystem.h"
#include "navMesh.h"
#include "lpoint3.h"
#include "pta_LVecBase3.h"
#include "lmatrix.h"


/**
 * NavMeshBuilder class contains all the vertices and triangles. It also 
 * has the functions to build the navigation meshes using those vertices and 
 * triangles. Set the properties of the actor and environment using this class 
 * and then build it to get a NavMesh object as output.
 */
class EXPCL_NAVMESHGEN NavMeshBuilder {
PUBLISHED:

  explicit NavMeshBuilder(NodePath parent = NodePath());
  PT(NavMesh) build();

  INLINE float get_actor_height() const;
  INLINE float get_actor_radius() const;
  INLINE float get_actor_max_climb() const;
  INLINE float get_actor_max_slope() const;
  INLINE float get_region_min_size() const;
  INLINE float get_region_merge_size() const;
  INLINE float get_edge_max_len() const;
  INLINE float get_edge_max_error() const;
  INLINE float get_verts_per_poly() const;
  INLINE float get_cell_size() const;
  INLINE float get_cell_height() const;
  INLINE float get_tile_size() const;
  INLINE NavMeshParams::PartitionType get_partition_type() const;

  INLINE void set_actor_height(float height);
  INLINE void set_actor_radius(float radius);
  INLINE void set_actor_max_climb(float climb);
  INLINE void set_actor_max_slope(float slope);
  INLINE void set_region_min_size(float region_min_size);
  INLINE void set_region_merge_size(float region_merge_size);
  INLINE void set_edge_max_len(float max_len);
  INLINE void set_edge_max_error(float max_error);
  INLINE void set_verts_per_poly(float verts_per_poly);
  INLINE void set_cell_size(float cs);
  INLINE void set_cell_height(float ch);
  INLINE void set_tile_size(float cs);
  INLINE void set_partition_type(NavMeshParams::PartitionType partition);

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

  void reset_common_settings();
  bool from_node_path(NodePath node, bool tracked_node = false);
  bool from_coll_node_path(NodePath node, BitMask32 mask = BitMask32::all_on(), bool tracked_node = false);
  bool from_geom(PT(Geom) geom);
  PT(GeomNode) draw_poly_mesh_geom();

  INLINE size_t get_vert_count() const;
  INLINE size_t get_tri_count() const;
  INLINE int get_pmesh_vert_count() const;
  INLINE int get_pmesh_poly_count() const;
  INLINE float get_build_time_ms() const;

  MAKE_PROPERTY(vert_count, get_vert_count);
  MAKE_PROPERTY(tri_count, get_tri_count);
  MAKE_PROPERTY(build_time_ms, get_build_time_ms);

  void add_polygon(LPoint3 a, LPoint3 b, LPoint3 c);
  void add_polygon(PTA_LVecBase3f &vec);

private:
  NodePath _parent;

  float _scale;
  std::vector<float> _verts;
  std::vector<int> _tris;

  void add_vertex(float x, float y, float z);
  void add_triangle(int a, int b, int c);

  void process_coll_node_path(NodePath &node, CPT(TransformState) &transform, BitMask32 mask, bool tracked_node);
  void process_node_path(NodePath &node, CPT(TransformState) &transform, bool tracked_node);
  void process_geom_node(PT(GeomNode) &geomnode, CPT(TransformState) &transform, bool tracked_node);
  void process_geom(CPT(Geom) &geom, const CPT(TransformState) &transform, bool tracked_node);
  void process_primitive(const GeomPrimitive *orig_prim, const GeomVertexData *vdata, LMatrix4 &transform, bool tracked_node);

  INLINE void update_bounds(LVector3 vert);

  void get_vert_tris(std::vector<float> &verts, std::vector<int> &tris);

  std::unordered_map<LVector3, int> _vertex_map;
  pvector<LVector3> _vertex_vector;
  pvector<TriVertGroup> _tri_verticies;
  bool _loaded{};
  int index_temp;

  LMatrix4 mat_from_y = LMatrix4::convert_mat(CS_yup_right, CS_default);
  LMatrix4 mat_to_y = LMatrix4::convert_mat(CS_default, CS_yup_right);

protected:
  PT(NavMesh) _nav_mesh_obj;

  float _cell_size;
  float _cell_height;
  float _tile_size;
  int _max_tiles;
  int _max_polys_per_tile;
  float _agent_height;
  float _agent_radius;
  float _agent_max_climb;
  float _agent_max_slope;
  float _region_min_size;
  float _region_merge_size;
  float _edge_max_len;
  float _edge_max_error;
  float _verts_per_poly;
  float _detail_sample_dist;
  float _detail_sample_max_error;
  NavMeshParams::PartitionType _partition_type;

  bool _filter_low_hanging_obstacles;
  bool _filter_ledge_spans;
  bool _filter_walkable_low_height_spans;

  bool _bounds_set = false;
  float _mesh_bMin[3] = { 0, 0, 0 };
  float _mesh_bMax[3] = { 0, 0, 0 };

  rcContext *_ctx = nullptr;

  float _total_build_time_ms = -1;

  std::vector<unsigned char> _triareas;
  rcHeightfield *_solid = nullptr;
  rcCompactHeightfield *_chf = nullptr;
  rcContourSet *_cset = nullptr;
  rcPolyMesh *_pmesh = nullptr;
  rcConfig _cfg = {};
  rcPolyMeshDetail *_dmesh = nullptr;

  TriVertGroups _untracked_tris;
  NodePaths _tracked_nodes;
  TrackedCollInfos _tracked_coll_nodes;

  void cleanup();

  unsigned char* buildTileMesh(int tx, int ty, const float* bmin, const float* bmax, int& dataSize, std::vector<float> &verts, std::vector<int> &tris);

public:

  ~NavMeshBuilder();
  void set_context(rcContext *ctx) { _ctx = ctx; }

  std::vector<float> get_verts() { return _verts; }
  std::vector<int> get_tris() { return _tris; }

  bool loaded_geom() const { return _loaded; }

};

#include "navMeshBuilder.I"

#endif // NAVMESHBUILDER_H
