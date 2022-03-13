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
#include "navMeshPoly.h"
#include <unordered_map>

/**
 * NavMeshParams class stores all the parameters of a navigation mesh.
 */
class EXPCL_NAVIGATION NavMeshParams {
PUBLISHED:
  int vert_count;
  int poly_count;
  int nvp;
  int detail_vert_count;
  int detail_tri_count;
  float walkable_height;
  float walkable_radius;
  float walkable_climb;
  float cs;
  float ch;
  bool build_bv_tree;
  float b_min[3];
  float b_max[3];
  unsigned short *verts;        // size: 3 * vert_count
  unsigned short *polys;        // size: poly_count * 2 * nvp
  unsigned short *poly_flags;   // size: poly_count
  unsigned char *poly_areas;    // size: poly_count
  unsigned int *detail_meshes;  // size: poly_count * 4
  float *detail_verts;          // size: detail_vert_count * 3
  unsigned char *detail_tris;   // size: detail_tri_count * 4
  int border_index = 65535;
  int input_model_coordinate_system = 0;

};

typedef pvector<NavMeshPoly> NavMeshPolys;


/**
 * NavMesh class stores the navigation mesh. The navigation mesh 
 * can be obtained using NavMeshBuilder class or can be generated 
 * using the NavMeshParams class by the user. 
 */
class EXPCL_NAVIGATION NavMesh : public TypedWritableReferenceCount
{
public:
  explicit NavMesh(dtNavMesh *nav_mesh);

PUBLISHED:
  explicit NavMesh(NavMeshParams mesh_params);
  NavMesh();
  PT(GeomNode) draw_nav_mesh_geom();
  INLINE int get_vert_count() const;
  INLINE int get_poly_count() const;
  INLINE int get_detail_vert_count() const;
  INLINE int get_detail_tri_count() const;

  NavMeshPolys get_polys();

  NavMeshPoly get_poly_at(LPoint3 point);

  NavMeshPolys get_polys_around(LPoint3 point, LVector3 extents = LVector3( 3 , 3 , 3 ));

  INLINE void reset_debug_colors();

private:
  dtNavMesh *_nav_mesh;
  dtNavMeshCreateParams _params {};
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
};

#include "navMesh.I"

#endif // NAVMESH_H
