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

#include "DetourNavMesh.h"
#include "DetourNavMeshBuilder.h"
#include "typedWritableReferenceCount.h"
#include "pandaFramework.h"
#include "pandaSystem.h"

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
  int RC_MESH_NULL_IDX = 65535;
};

class EXPCL_NAVIGATION NavMesh: public TypedWritableReferenceCount
{
PUBLISHED:
  NavMesh(dtNavMesh *nav_mesh);
  NavMesh(NavMeshParams mesh_params);
  void set_nav_mesh(dtNavMesh *m) { _nav_mesh = m; }
  NavMesh();
  PT(GeomNode) draw_nav_mesh_geom();

private:
  dtNavMesh *_nav_mesh;
  dtNavMeshCreateParams _params;
  int RC_MESH_NULL_IDX;
  
public:
  bool init_nav_mesh();
  dtNavMesh *get_nav_mesh() { return _nav_mesh; }
  ~NavMesh();
  

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



#endif // NAVMESH_H
