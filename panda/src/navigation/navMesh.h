/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navMesh.h
 * @author ashwini
 * @date 2020-060-21
 */


#ifndef NAVMESH_H
#define NAVMESH_H

#include "recastnavigation/DetourNavMesh.h"
#include "recastnavigation/DetourNavMeshBuilder.h"
#include "typedWritableReferenceCount.h"
#include "pandaSystem.h"
#include "lmatrix.h"
#include "eventParameter.h"
#include "pandabase.h"
#include "geomNode.h"
#include "nodePath.h"
#include "navMeshPoly.h"
#include <unordered_map>
#include <set>

/**
 * NavMeshParams class stores all the parameters of a navigation mesh.
 */
class EXPCL_NAVIGATION NavMeshParams {
PUBLISHED:
  enum PartitionType {
    SAMPLE_PARTITION_WATERSHED,
    SAMPLE_PARTITION_MONOTONE,
    SAMPLE_PARTITION_LAYERS,
  };

  float cell_size;
  float cell_height;
  float tile_size;
  int max_tiles;
  int max_polys_per_tile;
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
  float orig_bound_min[3];
  float tile_cell_size;
  PartitionType partition_type;
  bool filter_low_hanging_obstacles;
  bool filter_ledge_spans;
  bool filter_walkable_low_height_spans;
};


struct TriVertGroup {
  LVector3 a;
  LVector3 b;
  LVector3 c;
};

inline bool operator==(const TriVertGroup& lhs, const TriVertGroup& rhs)
{
  return std::tie(lhs.a, lhs.b, lhs.c) ==
         std::tie(rhs.a, rhs.b, rhs.c);
}

inline bool operator<(const TriVertGroup& lhs, const TriVertGroup& rhs)
{
  return std::tie(lhs.a, lhs.b, lhs.c) <
         std::tie(rhs.a, rhs.b, rhs.c);
}

struct TrackedCollInfo {
  NodePath node;
  BitMask32 mask;
};

typedef pvector<NavMeshPoly> NavMeshPolys;
typedef pvector<NodePath> NodePaths;
typedef pvector<TriVertGroup> TriVertGroups;
typedef pvector<TrackedCollInfo> TrackedCollInfos;

class NavMeshBuilder;

/**
 * NavMesh class stores the navigation mesh. The navigation mesh 
 * can be obtained using NavMeshBuilder class or can be generated 
 * using the NavMeshParams class by the user. 
 */
class EXPCL_NAVIGATION NavMesh : public TypedWritableReferenceCount
{
public:
  explicit NavMesh(dtNavMesh *nav_mesh,
                   NavMeshParams params,
                   TriVertGroups &untracked_tris,
                   NodePaths &tracked_nodes,
                   TrackedCollInfos &tracked_coll_nodes,
                   pvector<TriVertGroup> &last_tris);

PUBLISHED:
  explicit NavMesh(NavMeshParams mesh_params);
  NavMesh();
  PT(GeomNode) draw_nav_mesh_geom();

  NavMeshPolys get_polys();

  NavMeshPoly get_poly_at(LPoint3 point);

  NavMeshPolys get_polys_around(LPoint3 point, LVector3 extents = LVector3( 3 , 3 , 3 ));

  INLINE void reset_debug_colors();

  INLINE NavMeshParams get_params();

  INLINE NodePaths get_tracked_nodes();

  INLINE TrackedCollInfos get_tracked_coll_nodes();

  INLINE TriVertGroups get_untracked_tris();

  void update();

private:
  dtNavMesh *_nav_mesh;
  int border_index = 0;

  std::unordered_map<dtPolyRef, LColor> _debug_colors;

  PT(GeomNode) _cache_poly_outlines = nullptr;
  std::unordered_map<dtPolyRef, PointList> _cache_poly_verts;

  LMatrix4 mat_from_y = LMatrix4::convert_mat(CS_yup_right, CS_default);
  LMatrix4 mat_to_y = LMatrix4::convert_mat(CS_default, CS_yup_right);
  
public:
  bool init_nav_mesh();
  dtNavMesh *get_nav_mesh() { return _nav_mesh; }
  ~NavMesh();

  INLINE LColor get_poly_debug_color(dtPolyRef poly) const;
  INLINE void set_poly_debug_color(dtPolyRef poly, LColor color);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "NavMesh",
      TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() { init_type(); return get_class_type(); }

private:
  static TypeHandle _type_handle;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

  unsigned char* buildTileMesh(int tx, int ty, const float* bmin, const float* bmax, int& dataSize, pvector<float> &verts, pvector<int> &tris) const;

  TriVertGroups _untracked_tris;
  NodePaths _tracked_nodes;
  TrackedCollInfos _tracked_coll_nodes;
  NavMeshParams _params;
  std::set<TriVertGroup> _last_tris;
};

#include "navMesh.I"

#endif // NAVMESH_H
