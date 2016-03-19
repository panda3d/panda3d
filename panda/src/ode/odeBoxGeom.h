/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeBoxGeom.h
 * @author joswilso
 * @date 2006-12-27
 */

#ifndef ODEBOXGEOM_H
#define ODEBOXGEOM_H

#include "pandabase.h"
#include "typedObject.h"
#include "luse.h"

#include "ode_includes.h"
#include "odeGeom.h"

/**
 *
 */
class EXPCL_PANDAODE OdeBoxGeom : public OdeGeom {
  friend class OdeGeom;

public:
  OdeBoxGeom(dGeomID id);

PUBLISHED:
  OdeBoxGeom(dReal lx, dReal ly, dReal lz);
  OdeBoxGeom(OdeSpace &space, dReal lx, dReal ly, dReal lz);
  OdeBoxGeom(OdeSpace &space, const LVecBase3f &size);
  virtual ~OdeBoxGeom();

  INLINE void set_lengths(dReal lx, dReal ly, dReal lz);
  INLINE void set_lengths(const LVecBase3f &size);
  INLINE LVecBase3f get_lengths();
  INLINE dReal get_point_depth(dReal x, dReal y, dReal z);
  INLINE dReal get_point_depth(const LPoint3f &p);

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
