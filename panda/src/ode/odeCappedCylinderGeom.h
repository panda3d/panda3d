// Filename: odeCappedCylinderGeom.h
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

#ifndef ODECAPPEDCYLINDERGEOM_H
#define ODECAPPEDCYLINDERGEOM_H

#include "pandabase.h"
#include "typedObject.h"
#include "luse.h"

#include "ode_includes.h"
#include "odeGeom.h"

////////////////////////////////////////////////////////////////////
//       Class : OdeCappedCylinderGeom
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAODE OdeCappedCylinderGeom : public OdeGeom {
PUBLISHED:
  OdeCappedCylinderGeom(dReal radius, dReal length);
  OdeCappedCylinderGeom(OdeSpace &space, dReal radius, dReal length);
  virtual ~OdeCappedCylinderGeom();

  INLINE void set_params(dReal radius, dReal length);
  INLINE void get_params(dReal *radius, dReal *length) const;
  INLINE dReal get_radius() const;
  INLINE dReal get_length() const;
  INLINE dReal get_point_depth(dReal x, dReal y, dReal z) const;

public:
  INLINE static int get_geom_class() { return dCapsuleClass; };

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    OdeGeom::init_type();
    register_type(_type_handle, "OdeCappedCylinderGeom",
		  OdeGeom::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "odeCappedCylinderGeom.I"

#endif
