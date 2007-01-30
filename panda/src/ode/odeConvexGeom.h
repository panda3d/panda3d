// Filename: odeConvexGeom.h
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
