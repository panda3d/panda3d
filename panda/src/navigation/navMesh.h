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
#include "navMeshBuilder.h"
#include "navTriVertGroup.h"
#include "navTrackedCollInfo.h"
#include "navMeshParams.h"


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
                   std::set<NavTriVertGroup> &untracked_tris);

PUBLISHED:
  explicit NavMesh(NavMeshParams mesh_params);
  NavMesh();
  PT(GeomNode) draw_nav_mesh_geom();

  NavMeshPolys get_polys();

  NavMeshPoly get_poly_at(LPoint3 point);

  NavMeshPolys get_polys_around(LPoint3 point, LVector3 extents = LVector3( 3 , 3 , 3 ));

  INLINE void reset_debug_colors();

  void add_node_path(NodePath node, bool tracked = true);
  void add_coll_node_path(NodePath node, BitMask32 mask = BitMask32::all_on(), bool tracked = true);
  void add_geom(PT(Geom) geom);

  INLINE const NavMeshParams& get_params() const;
  INLINE const NodePaths& get_tracked_nodes() const;
  INLINE const TrackedCollInfos& get_tracked_coll_nodes() const;
  INLINE const NavTriVertGroups& get_untracked_tris() const;

  MAKE_PROPERTY(params, get_params);
  MAKE_PROPERTY(tracked_nodes, get_tracked_nodes);
  MAKE_PROPERTY(tracked_coll_nodes, get_tracked_coll_nodes);
  MAKE_PROPERTY(untracked_tris, get_untracked_tris);

  void update();

private:
  dtNavMesh *_nav_mesh;
  int border_index = 0;

  NavTriVertGroups _untracked_tris;
  NodePaths _tracked_nodes;
  TrackedCollInfos _tracked_coll_nodes;
  NavMeshParams _params;
  NavMeshBuilder _internal_rebuilder;

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

};

#include "navMesh.I"

#endif // NAVMESH_H
