/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeTriMeshGeom.h
 * @author joswilso
 * @date 2006-12-27
 */

#ifndef ODETRIMESHGEOM_H
#define ODETRIMESHGEOM_H

#include "pandabase.h"
#include "luse.h"

#include "ode_includes.h"
#include "odeGeom.h"
#include "odeTriMeshData.h"

/**
 *
 */
class EXPCL_PANDAODE OdeTriMeshGeom : public OdeGeom {
  friend class OdeGeom;

public:
  OdeTriMeshGeom(dGeomID id);

PUBLISHED:
  /* ODE_API dGeomID dCreateTriMesh(dSpaceID space, dTriMeshDataID Data, dTriCallback* Callback, dTriArrayCallback* ArrayCallback, dTriRayCallback* RayCallback); */
  OdeTriMeshGeom(OdeTriMeshData &data);
  OdeTriMeshGeom(OdeSpace &space, OdeTriMeshData &data);
  OdeTriMeshGeom(const OdeTriMeshGeom &copy);
  virtual ~OdeTriMeshGeom();

  void destroy();
  INLINE void set_tri_mesh_data(OdeTriMeshData &data);
  INLINE PT(OdeTriMeshData) get_tri_mesh_data() const;
  INLINE void set_data(OdeTriMeshData &data);
  INLINE PT(OdeTriMeshData) get_data() const;
  INLINE void enable_TC(int geom_class, int enable);
  INLINE int is_TC_enabled(int geom_class) const;
  INLINE void clear_TC_cache(const OdeGeom &geom);
  INLINE void get_triangle(int face_index, LPoint3f &v0, LPoint3f &v1, LPoint3f &v2) const;
  INLINE LPoint3f get_point(int face_index, dReal u, dReal v) const;
  INLINE int get_num_triangles() const;

public:
  INLINE static int get_geom_class() { return dTriMeshClass; };
  INLINE dTriMeshDataID get_tri_mesh_data_id() const;
  INLINE dTriMeshDataID get_data_id() const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    OdeGeom::init_type();
    register_type(_type_handle, "OdeTriMeshGeom",
                  OdeGeom::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "odeTriMeshGeom.I"

#endif
