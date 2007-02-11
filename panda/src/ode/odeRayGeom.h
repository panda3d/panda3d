// Filename: odeRayGeom.h
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

#ifndef ODERAYGEOM_H
#define ODERAYGEOM_H

#include "pandabase.h"
#include "luse.h"

#include "ode_includes.h"
#include "odeGeom.h"

////////////////////////////////////////////////////////////////////
//       Class : OdeRayGeom
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAODE OdeRayGeom : public OdeGeom {
  friend class OdeGeom;

private:
  OdeRayGeom(dGeomID id);

PUBLISHED:
  OdeRayGeom(dReal length);
  OdeRayGeom(OdeSpace &space, dReal length);
  virtual ~OdeRayGeom();

  INLINE void set_length(dReal length);
  INLINE dReal get_length();
  INLINE void set(dReal px, dReal py, dReal pz, dReal dx, dReal dy, dReal dz);
  INLINE void set(const LVecBase3f &start, const LVecBase3f &dir);
  INLINE void get(LVecBase3f &start, LVecBase3f &dir) const;
  INLINE LVecBase3f get_start() const;
  INLINE LVecBase3f get_direction() const;
  INLINE void set_params(int first_contact, int backface_cull);
  INLINE void get_params(int &first_contact, int &backface_cull) const;
  INLINE int get_first_contact() const;
  INLINE int get_backface_cull() const;
  INLINE void set_closest_hit(int closest_hit);
  INLINE int get_closest_hit();

public:
  INLINE static int get_geom_class() { return dRayClass; };

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    OdeGeom::init_type();
    register_type(_type_handle, "OdeRayGeom",
		  OdeGeom::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "odeRayGeom.I"

#endif
