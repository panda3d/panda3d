/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odePlaneGeom.h
 * @author joswilso
 * @date 2006-12-27
 */

#ifndef ODEPLANEGEOM_H
#define ODEPLANEGEOM_H

#include "pandabase.h"
#include "luse.h"

#include "ode_includes.h"
#include "odeGeom.h"

/**
 *
 */
class EXPCL_PANDAODE OdePlaneGeom : public OdeGeom {
  friend class OdeGeom;

public:
  OdePlaneGeom(dGeomID id);

PUBLISHED:
  OdePlaneGeom(dReal a, dReal b, dReal c, dReal d);
  OdePlaneGeom(const LVecBase4f &params);
  OdePlaneGeom(OdeSpace &space, dReal a, dReal b, dReal c, dReal d);
  OdePlaneGeom(OdeSpace &space, const LVecBase4f &params);
  virtual ~OdePlaneGeom();

  INLINE void set_params(dReal a, dReal b, dReal c, dReal d);
  INLINE void set_params(const LVecBase4f &params);
  INLINE LVecBase4f get_params() const;
  INLINE dReal get_point_depth(dReal x, dReal y, dReal z) const;
  INLINE dReal get_point_depth(const LPoint3f &p) const;

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
