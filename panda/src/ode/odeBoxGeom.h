// Filename: odeBoxGeom.h
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

#ifndef ODEBOXGEOM_H
#define ODEBOXGEOM_H

#include "pandabase.h"
#include "typedObject.h"
#include "luse.h"

#include "ode_includes.h"
#include "odeGeom.h"

////////////////////////////////////////////////////////////////////
//       Class : OdeBoxGeom
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAODE OdeBoxGeom : public OdeGeom {
PUBLISHED:
  OdeBoxGeom(dReal lx, dReal ly, dReal lz);
  OdeBoxGeom(OdeSpace &space, dReal lx, dReal ly, dReal lz);
  virtual ~OdeBoxGeom();

  INLINE void set_lengths(dReal lx, dReal ly, dReal lz);
  INLINE LVecBase3f get_lengths();
  INLINE dReal get_point_depth(dReal x, dReal y, dReal z);

public:
  INLINE static int get_geom_class() { return dBoxClass; };

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    OdeGeom::init_type();
    register_type(_type_handle, "OdeBoxGeom",
		  OdeGeom::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "odeBoxGeom.I"

#endif
