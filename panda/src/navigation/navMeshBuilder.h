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
#include "recastnavigation/DetourTileCache.h"
#include <string>
#include <unordered_map>
#include <tuple>
#include "geom.h"
#include "geomNode.h"
#include "nodePath.h"
#include "pandaSystem.h"
#include "lpoint3.h"
#include "pta_LVecBase3.h"
#include "lmatrix.h"
#include "navMeshParams.h"
#include "navTriVertGroup.h"


class NavMesh;
struct ExpressCompressor;
struct LinearAllocator;
struct MeshProcess;
struct TileCacheData;


/**
 * NavMeshBuilder class contains all the vertices and triangles. It also 
 * has the functions to build the navigation meshes using those vertices and 
 * triangles. Set the properties of the actor and environment using this class 
 * and then build it to get a NavMesh object as output.
 */
class EXPCL_NAVIGATION NavMeshBuilder {
PUBLISHED:

  explicit NavMeshBuilder(NodePath parent = NodePath());
  PT(NavMesh) build();

  bool from_node_path(NodePath node);
  bool from_coll_node_path(NodePath node, BitMask32 mask = BitMask32::all_on());
  bool from_geom(PT(Geom) geom);
  PT(GeomNode) draw_poly_mesh_geom();

  INLINE int get_pmesh_vert_count() const;
  INLINE int get_pmesh_poly_count() const;
  INLINE float get_build_time_ms() const;

  INLINE NavMeshParams& get_params();
  INLINE void set_params(NavMeshParams &params);

  MAKE_PROPERTY(build_time_ms, get_build_time_ms);
  MAKE_PROPERTY(params, get_params, set_params);

  void add_polygon(LPoint3 a, LPoint3 b, LPoint3 c);
  void add_polygon(PTA_LVecBase3f &vec);

public:
  struct ObstacleData {
    int type;
    LPoint3 a;
    LPoint3 b;
    float c;
    float d;

#ifndef CPPPARSER
    // Interrogate doesn't like tuples so just hide them.
    std::tuple<const int &, const LPoint3f &, const LPoint3f &, const float &, const float &> tie() const { return std::tie(type, a, b, c, d); }
    inline bool operator==(const ObstacleData& other) const
    {
      return tie() == other.tie();
    }

    inline bool operator<(const ObstacleData& other) const
    {
      return tie() < other.tie();
    }
#endif
  };

private:
  NodePath _parent;

  NavMeshParams _params;

  void process_coll_node_path(std::set<NavTriVertGroup> &tris, const NodePath &node, CPT(TransformState) &transform, BitMask32 mask);
  void process_node_path(std::set<NavTriVertGroup> &tris, const NodePath &node, CPT(TransformState) &transform);
  void process_geom_node(std::set<NavTriVertGroup> &tris, PT(GeomNode) &geomnode, CPT(TransformState) &transform);
  void process_geom(std::set<NavTriVertGroup> &tris, CPT(Geom) &geom, const CPT(TransformState) &transform);
  void process_primitive(std::set<NavTriVertGroup> &tris, const GeomPrimitive *orig_prim, const GeomVertexData *vdata, LMatrix4 &transform);

  void process_obstacle_node_path(std::set<ObstacleData> &existing_obstacles, std::set<ObstacleData> &new_obstacles, const NodePath &node, CPT(TransformState) &transform);

  INLINE void update_bounds(LVector3 vert);

  INLINE static void get_vert_tris(const std::set<NavTriVertGroup> &tri_vert_groups, pvector<float> &verts, pvector<int> &tris);

  bool _loaded{};
  int index_temp;

  LMatrix4 mat_from_y = LMatrix4::convert_mat(CS_yup_right, CS_default);
  LMatrix4 mat_to_y = LMatrix4::convert_mat(CS_default, CS_yup_right);

protected:
  PT(NavMesh) _nav_mesh_obj;
  std::shared_ptr<dtTileCache> _tile_cache;
  std::shared_ptr<struct MeshProcess> _tile_mesh_proc;
  std::shared_ptr<struct ExpressCompressor> _tile_compressor;
  std::shared_ptr<struct LinearAllocator> _tile_alloc;

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

  std::set<NavTriVertGroup> _untracked_tris;
  std::set<NavTriVertGroup> _last_tris;

  void cleanup();

  unsigned char* buildTileMesh(int tx, int ty, const float* bmin, const float* bmax, int& dataSize, pvector<float> &verts, pvector<int> &tris);

  explicit NavMeshBuilder(PT(NavMesh) navMesh);

  void update_nav_mesh();

  int rasterizeTileLayers(const int tx, const int ty,
                          const float* bmin, const float* bmax,
                          pvector<float> &verts, pvector<int> &tris,
                          TileCacheData *tiles, const int maxTiles);

  friend class NavMesh;

public:
  ~NavMeshBuilder();

  void set_context(rcContext *ctx) { _ctx = ctx; }

  bool loaded_geom() const { return _loaded; }
};

#include "navMeshBuilder.I"

#endif // NAVMESHBUILDER_H
