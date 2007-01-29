// Filename: odeTriMeshGeom.h
// Created by:  joswilso (27Dec06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef ODETRIMESHGEOM_H
#define ODETRIMESHGEOM_H

#include "pandabase.h"
#include "luse.h"

#include "ode/ode.h"
#include "odeGeom.h"
#include "odeTriMeshData.h"

////////////////////////////////////////////////////////////////////
//       Class : OdeTriMeshGeom
// Description : 
////////////////////////////////////////////////////////////////////

typedef pmap<dGeomID, PT(OdeTriMeshData)> TriMeshDataMap;

class EXPCL_PANDAODE OdeTriMeshGeom : public OdeGeom {
protected:
  static TriMeshDataMap MeshData;

PUBLISHED:
  //ODE_API dGeomID dCreateTriMesh(dSpaceID space, dTriMeshDataID Data, dTriCallback* Callback, dTriArrayCallback* ArrayCallback, dTriRayCallback* RayCallback);
  OdeTriMeshGeom(OdeTriMeshData &data);
  OdeTriMeshGeom(OdeSpace &space, OdeTriMeshData &data);
  OdeTriMeshGeom(OdeTriMeshGeom &copy);
  virtual ~OdeTriMeshGeom();

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
  INLINE dTriMeshDataID get_data_id() const;
  static void link_data(dGeomID, OdeTriMeshData &data);
  static void unlink_data(dGeomID);

private:
  void operator = (const OdeTriMeshGeom &copy);

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
