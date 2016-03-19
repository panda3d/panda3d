/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeSphereGeom.h
 * @author joswilso
 * @date 2006-12-27
 */

#ifndef ODESPHEREGEOM_H
#define ODESPHEREGEOM_H

#include "pandabase.h"
#include "luse.h"

#include "ode_includes.h"
#include "odeGeom.h"

/**
 *
 */
class EXPCL_PANDAODE OdeSphereGeom : public OdeGeom {
  friend class OdeGeom;

public:
  OdeSphereGeom(dGeomID id);

PUBLISHED:
  OdeSphereGeom(dReal radius);
  OdeSphereGeom(OdeSpace &space, dReal radius);
  OdeSphereGeom(OdeGeom &geom);
  virtual ~OdeSphereGeom();

  INLINE void set_radius(dReal radius);
  INLINE dReal get_radius() const;
  INLINE dReal get_point_depth(dReal x, dReal y, dReal z) const;
  INLINE dReal get_point_depth(const LPoint3f &p) const;

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
