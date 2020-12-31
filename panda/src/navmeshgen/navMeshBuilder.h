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

#include "Recast.h"
#include <string>
#include "geom.h"
#include "geomNode.h"
#include "nodePath.h"
#include "pandaFramework.h"
#include "pandaSystem.h"
#include "navMesh.h"
#include "lpoint3.h"
#include "pta_LVecBase3.h"
#include "lmatrix.h"

enum SamplePolyAreas {
  SAMPLE_POLYAREA_GROUND,
  SAMPLE_POLYAREA_WATER,
  SAMPLE_POLYAREA_ROAD,
  SAMPLE_POLYAREA_DOOR,
  SAMPLE_POLYAREA_GRASS,
  SAMPLE_POLYAREA_JUMP
};

enum SamplePolyFlags {
  SAMPLE_POLYFLAGS_WALK = 0x01,		// Ability to walk (ground, grass, road)
  SAMPLE_POLYFLAGS_SWIM = 0x02,		// Ability to swim (water).
  SAMPLE_POLYFLAGS_DOOR = 0x04,		// Ability to move through doors.
  SAMPLE_POLYFLAGS_JUMP = 0x08,		// Ability to jump.
  SAMPLE_POLYFLAGS_DISABLED = 0x10,		// Disabled polygon
  SAMPLE_POLYFLAGS_ALL = 0xffff	// All abilities.
};

/**
 * NavMeshBuilder class contains all the vertices and triangles. It also 
 * has the functions to build the navigation meshes using those vertices and 
 * triangles. Set the properties of the actor and environment using this class 
 * and then build it to get a NavMesh object as output.
 */
class EXPCL_NAVMESHGEN NavMeshBuilder {
PUBLISHED:
  enum PartitionType {
    SAMPLE_PARTITION_WATERSHED,
    SAMPLE_PARTITION_MONOTONE,
    SAMPLE_PARTITION_LAYERS,
  };
  
  float get_actor_radius() { return _agent_radius; }
  float get_actor_height() { return _agent_height; }
  float get_actor_max_climb() { return _agent_max_climb; }
  float get_actor_max_slope() { return _agent_max_slope; }
  float get_region_min_size() { return _region_min_size; }
  float get_region_merge_size() { return _region_merge_size; }
  float get_edge_max_len() { return _edge_max_len; }
  float get_edge_max_error() { return _edge_max_error; }
  float get_verts_per_poly() { return _verts_per_poly; }
  float get_cell_size() { return _cell_size; }
  float get_cell_height() { return _cell_height; }
  PartitionType get_partition_type() { return _partition_type; }
  
  void set_actor_height(float height) { _agent_height = height; }
  void set_actor_radius(float radius) { _agent_radius = radius; }
  void set_actor_max_climb(float climb) { _agent_max_climb = climb; }
  void set_actor_max_slope(float slope) { _agent_max_slope = slope; }
  void set_region_min_size(float region_min_size) { _region_min_size = region_min_size; }
  void set_region_merge_size(float region_merge_size) { _region_merge_size = region_merge_size; }
  void set_edge_max_len(float max_len) { _edge_max_len = max_len; }
  void set_edge_max_error(float max_error) { _edge_max_error = max_error; }
  void set_verts_per_poly(float verts_per_poly) { _verts_per_poly = verts_per_poly; }
  void set_cell_size(float cs) { _cell_size = cs; }
  void set_cell_height(float ch) { _cell_height = ch; }
  void set_partition_type(PartitionType partition) { _partition_type = partition; }
  
  MAKE_PROPERTY(_agent_radius, get_actor_radius, set_actor_radius);
  MAKE_PROPERTY(_agent_height, get_actor_height, set_actor_height);
  MAKE_PROPERTY(_agent_max_climb, get_actor_max_climb, set_actor_max_climb);
  MAKE_PROPERTY(_agent_max_slope, get_actor_max_slope, set_actor_max_slope);
  MAKE_PROPERTY(_region_min_size, get_region_min_size, set_region_min_size);
  MAKE_PROPERTY(_region_merge_size, get_region_merge_size, set_region_merge_size);
  MAKE_PROPERTY(_edge_max_len, get_edge_max_len, set_edge_max_len);
  MAKE_PROPERTY(_edge_max_error, get_edge_max_error, set_edge_max_error);
  MAKE_PROPERTY(_verts_per_poly, get_verts_per_poly, set_verts_per_poly);
  MAKE_PROPERTY(_cell_size, get_cell_size, set_cell_size);
  MAKE_PROPERTY(_cell_height, get_cell_height, set_cell_height);
  MAKE_PROPERTY(_partition_type, get_partition_type, set_partition_type);
  
  void reset_common_settings();
  bool from_node_path(NodePath node);
  bool from_geom(PT(Geom) geom);
  PT(GeomNode) draw_poly_mesh_geom();

  PT(NavMesh) build();
  NavMeshBuilder();

  int get_vert_count() const { return _vert_count; }
  int get_tri_count() const { return _tri_count; }
  int get_pmesh_vert_count() { return _pmesh->nverts; }
  int get_pmesh_poly_count() { return _pmesh->npolys; }
  int get_pmesh_max_poly_count() { return _pmesh->maxpolys; }

  void add_polygon(LPoint3 a, LPoint3 b, LPoint3 c);
  void add_polygon(PTA_LVecBase3f vec);

private:
  float _scale;
  float *_verts;
  int *_tris;
  float *_normals;
  int _vert_count;
  int _tri_count;

  void add_vertex(float x, float y, float z, int &cap);
  void add_triangle(int a, int b, int c, int &cap);

  void process_geom_node(GeomNode *geomnode, int &vcap, int &tcap);
  void process_geom(CPT(Geom) geom, int &vcap, int &tcap);
  void process_vertex_data(const GeomVertexData *vdata, int &vcap);
  void process_primitive(const GeomPrimitive *orig_prim, const GeomVertexData *vdata, int &tcap);

  std::map<LVector3, int> _vertex_map;
  std::vector<LVector3> _vertex_vector, _face_vector;
  int _vert_capacity, _tri_capacity;
  bool _loaded;
  int index_temp;

  LMatrix4 mat_from_y = LMatrix4::convert_mat(CS_yup_right, CS_default);
  LMatrix4 mat_from_z = LMatrix4::convert_mat(CS_zup_right, CS_default);
  LMatrix4 mat_to_y = LMatrix4::convert_mat(CS_default, CS_yup_right);
  LMatrix4 mat_to_z = LMatrix4::convert_mat(CS_default, CS_zup_right);

protected:
  PT(NavMesh) _nav_mesh_obj;
  class dtNavMesh *_nav_mesh;

  unsigned char _nav_mesh_draw_flags;

  float _cell_size;
  float _cell_height;
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
  PartitionType _partition_type;
  
  bool _filter_low_hanging_obstacles;
  bool _filter_ledge_spans;
  bool _filter_walkable_low_height_spans;
  float _mesh_bMin[3], _mesh_bMax[3];

  rcContext *_ctx;

  float _total_build_time_ms;

  unsigned char *_triareas;
  rcHeightfield *_solid;
  rcCompactHeightfield *_chf;
  rcContourSet *_cset;
  rcPolyMesh *_pmesh;
  rcConfig _cfg;
  rcPolyMeshDetail *_dmesh;

  void cleanup();

public:
  
  ~NavMeshBuilder();
  void set_context(rcContext *ctx) { _ctx = ctx; }

  unsigned char get_nav_mesh_draw_flags() const { return _nav_mesh_draw_flags; }
  const float *get_verts() const { return _verts; }
  const float *get_normals() const { return _normals; }
  const int *get_tris() const { return _tris; }
  
  bool loaded_geom() { return _loaded; }

};

#endif // NAVMESHBUILDER_H
