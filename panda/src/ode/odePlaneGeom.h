// Filename: odePlaneGeom.h
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

#ifndef ODEPLANEGEOM_H
#define ODEPLANEGEOM_H

#include "pandabase.h"
#include "luse.h"

#include "ode/ode.h"
#include "odeGeom.h"

////////////////////////////////////////////////////////////////////
//       Class : OdePlaneGeom
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAODE OdePlaneGeom : public OdeGeom {
PUBLISHED:
  OdePlaneGeom(dReal a, dReal b, dReal c, dReal d);
  OdePlaneGeom(OdeSpace &space, dReal a, dReal b, dReal c, dReal d);
  virtual ~OdePlaneGeom();

  INLINE void set_params(dReal a, dReal b, dReal c, dReal d);
  INLINE LVecBase4f get_params() const;
  INLINE dReal get_point_depth(dReal x, dReal y, dReal z) const;

public:
  INLINE static int get_geom_class() { return dPlaneClass; };

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    OdeGeom::init_type();
    register_type(_type_handle, "OdePlaneGeom",
		  OdeGeom::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "odePlaneGeom.I"

#endif
