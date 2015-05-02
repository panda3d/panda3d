// Filename: odeCylinderGeom.h
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

#ifndef ODECYLINDERGEOM_H
#define ODECYLINDERGEOM_H

#include "pandabase.h"
#include "typedObject.h"
#include "luse.h"

#include "ode_includes.h"
#include "odeGeom.h"

////////////////////////////////////////////////////////////////////
//       Class : OdeCylinderGeom
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAODE OdeCylinderGeom : public OdeGeom {
  friend class OdeGeom;

public:
  OdeCylinderGeom(dGeomID id);

PUBLISHED:
  OdeCylinderGeom(dReal radius, dReal length);
  OdeCylinderGeom(OdeSpace &space, dReal radius, dReal length);
  virtual ~OdeCylinderGeom();

  INLINE void set_params(dReal radius, dReal length);
  INLINE void get_params(dReal *radius, dReal *length) const;
  INLINE dReal get_radius() const;
  INLINE dReal get_length() const;

public:
  INLINE static int get_geom_class() { return dCylinderClass; };

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    OdeGeom::init_type();
    register_type(_type_handle, "OdeCylinderGeom",
                  OdeGeom::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "odeCylinderGeom.I"

#endif
