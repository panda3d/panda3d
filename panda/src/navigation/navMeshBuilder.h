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


class dtTileCache;
struct dtTileCacheAlloc;
struct dtTileCacheCompressor;
struct dtTileCacheMeshProcess;

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
	explicit NavMeshBuilder(NavMeshParams &params, NodePath parent = NodePath());
  PT(NavMesh) build();

  bool add_node_path(NodePath node);
  bool add_coll_node_path(NodePath node, BitMask32 mask = BitMask32::all_on());
  bool add_geom(PT(Geom) geom);

  INLINE NavMeshParams& get_params();
  INLINE void set_params(NavMeshParams &params);
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

  static dtTileCacheAlloc *make_tile_allocator();
  static dtTileCacheMeshProcess *make_mesh_process();
  static dtTileCacheCompressor *make_tile_compressor();

private:
  NodePath _parent;

  NavMeshParams _params;

  void process_coll_node_path(std::set<NavTriVertGroup> &tris, const NodePath &node, CPT(TransformState) &transform, BitMask32 mask);
  void process_node_path(std::set<NavTriVertGroup> &tris, const NodePath &node, CPT(TransformState) &transform);
  void process_geom_node(std::set<NavTriVertGroup> &tris, PT(GeomNode) &geomnode, CPT(TransformState) &transform);
  void process_geom(std::set<NavTriVertGroup> &tris, CPT(Geom) &geom, const CPT(TransformState) &transform);
  void process_primitive(std::set<NavTriVertGroup> &tris, const GeomPrimitive *orig_prim, const GeomVertexData *vdata, LMatrix4 &transform);

  void process_obstacle_node_path(dtTileCache *tile_cache, std::set<ObstacleData> &existing_obstacles, std::set<ObstacleData> &new_obstacles, const NodePath &node, CPT(TransformState) &transform);

  INLINE void update_bounds(LVector3 vert);

  INLINE static void get_vert_tris(const std::set<NavTriVertGroup> &tri_vert_groups, pvector<float> &verts, pvector<int> &tris);

  LMatrix4 mat_from_y = LMatrix4::convert_mat(CS_yup_right, CS_default);
  LMatrix4 mat_to_y = LMatrix4::convert_mat(CS_default, CS_yup_right);

protected:
  bool _bounds_set = false;
  float _mesh_bMin[3] = { 0, 0, 0 };
  float _mesh_bMax[3] = { 0, 0, 0 };

  std::set<NavTriVertGroup> _untracked_tris;
  std::set<NavTriVertGroup> _last_tris;

  void update_nav_mesh(NavMesh *nav_mesh_obj, dtTileCache *tile_cache);

  int rasterizeTileLayers(const int tx, const int ty,
                          const float* bmin, const float* bmax,
                          pvector<float> &verts, pvector<int> &tris,
                          TileCacheData *tiles, const int maxTiles);

  friend class NavMesh;

public:
  ~NavMeshBuilder();
};

#include "navMeshBuilder.I"

#endif // NAVMESHBUILDER_H
