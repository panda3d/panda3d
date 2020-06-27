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
 * @date 2020-060-21
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

enum SamplePolyAreas {
  SAMPLE_POLYAREA_GROUND,
  SAMPLE_POLYAREA_WATER,
  SAMPLE_POLYAREA_ROAD,
  SAMPLE_POLYAREA_DOOR,
  SAMPLE_POLYAREA_GRASS,
  SAMPLE_POLYAREA_JUMP,
};
enum SamplePolyFlags {
  SAMPLE_POLYFLAGS_WALK = 0x01,		// Ability to walk (ground, grass, road)
  SAMPLE_POLYFLAGS_SWIM = 0x02,		// Ability to swim (water).
  SAMPLE_POLYFLAGS_DOOR = 0x04,		// Ability to move through doors.
  SAMPLE_POLYFLAGS_JUMP = 0x08,		// Ability to jump.
  SAMPLE_POLYFLAGS_DISABLED = 0x10,		// Disabled polygon
  SAMPLE_POLYFLAGS_ALL = 0xffff	// All abilities.
};
struct BuildSettings {
  
  float cell_size;
  float cell_height;
  float agent_height;
  float agent_radius;
  float agent_max_climb;
  float agent_max_slope;
  float region_min_size;
  float region_merge_size;
  float edge_max_len;
  float edge_max_error;
  float verts_per_poly;
  float detail_sample_dist;
  float detail_sample_max_error;
  int partition_type;
  float nav_mesh_bMin[3];
  float nav_mesh_bMax[3];
  float tile_size;
};


class EXPCL_NAVIGATION NavMeshBuilder {
PUBLISHED:
  float get_actor_radius() { return _agent_radius; }
  float get_actor_height() { return _agent_height; }
  float get_actor_climb() { return _agent_max_climb; }
  void set_actor_height(float h) { _agent_height = h; }
  void set_actor_radius(float r) { _agent_radius = r; }
  void set_actor_climb(float c) { _agent_max_climb = c; }
  void set_partition_type(std::string p);
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
private:
  std::string _filename;
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

  std::map<LVector3, int> vertex_map;
  std::vector<LVector3> vertex_vector, face_vector;
  bool _loaded;
  int index_temp;
protected:
  PT(NavMesh) _nav_mesh_obj;
  class dtNavMesh *_nav_mesh;
  class dtNavMeshQuery *_nav_query;
  class dtCrowd *_crowd;

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
  int _partition_type;

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

  virtual void collect_settings(struct BuildSettings &settings);

  
  dtNavMeshQuery *get_nav_query() { return _nav_query; }

 
  unsigned char getNavMeshDrawFlags() const { return _nav_mesh_draw_flags; }
  const float *get_verts() const { return _verts; }
  const float *get_normals() const { return _normals; }
  const int *get_tris() const { return _tris; }
  
  const std::string& get_file_name() const { return _filename; }
  bool loaded_geom() { return _loaded; }
  

};

enum SamplePartitionType {
  SAMPLE_PARTITION_WATERSHED,
  SAMPLE_PARTITION_MONOTONE,
  SAMPLE_PARTITION_LAYERS,
};


#endif // NAVMESHBUILDER_H
