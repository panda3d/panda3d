// Filename: odeSphereGeom.h
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

#ifndef ODESPHEREGEOM_H
#define ODESPHEREGEOM_H

#include "pandabase.h"
#include "luse.h"

#include "ode/ode.h"
#include "odeGeom.h"

////////////////////////////////////////////////////////////////////
//       Class : OdeSphereGeom
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAODE OdeSphereGeom : public OdeGeom {
PUBLISHED:
  OdeSphereGeom(dReal radius);
  OdeSphereGeom(OdeSpace &space, dReal radius);
  OdeSphereGeom(OdeGeom &geom);
  virtual ~OdeSphereGeom();

  INLINE void set_radius(dReal radius);
  INLINE dReal get_radius() const;
  INLINE dReal get_point_depth(dReal x, dReal y, dReal z) const;

public:
  INLINE static int get_geom_class() { return dSphereClass; };

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    OdeGeom::init_type();
    register_type(_type_handle, "OdeSphereGeom",
		  OdeGeom::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "odeSphereGeom.I"

#endif
