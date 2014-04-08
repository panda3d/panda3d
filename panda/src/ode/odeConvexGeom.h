// Filename: odeConvexGeom.h
// Created by:  joswilso (27Dec06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef ODECONVEXGEOM_H
#define ODECONVEXGEOM_H

#include "pandabase.h"
#include "typedObject.h"
#include "luse.h"

#include "ode_includes.h"
#include "odeGeom.h"

////////////////////////////////////////////////////////////////////
//       Class : OdeConvexGeom
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAODE OdeConvexGeom : public OdeGeom {
  friend class OdeGeom;

public:
  OdeConvexGeom(dGeomID id);

PUBLISHED:
  OdeConvexGeom();
  virtual ~OdeConvexGeom();

  INLINE void set_convex(dReal *_planes, unsigned int _count, dReal *_points, unsigned int _pointcount, unsigned int *_polygons);

public:
  INLINE static int get_geom_class() { return dConvexClass; };

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    OdeGeom::init_type();
    register_type(_type_handle, "OdeConvexGeom",
                  OdeGeom::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "odeConvexGeom.I"

#endif
