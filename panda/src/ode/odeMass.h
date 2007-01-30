// Filename: odeMass.h
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

#ifndef ODEMASS_H
#define ODEMASS_H

#include "pandabase.h"
#include "typedReferenceCount.h"
#include "luse.h"

#include "ode_includes.h"

////////////////////////////////////////////////////////////////////
//       Class : OdeMass
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAODE OdeMass : public TypedReferenceCount {
PUBLISHED:
  OdeMass();
  OdeMass(const OdeMass &copy);
  virtual ~OdeMass();
  
  INLINE int check();
  INLINE void set_zero();
  INLINE void set_parameters(dReal themass,
                             dReal cgx, dReal cgy, dReal cgz,
                             dReal I11, dReal I22, dReal I33,
                             dReal I12, dReal I13, dReal I23);
  INLINE void set_sphere(dReal density, dReal radius);
  INLINE void set_sphere_total(dReal total_mass, dReal radius);
  INLINE void set_capsule(dReal density, int direction,
                          dReal radius, dReal length);
  INLINE void set_capsule_total(dReal total_mass, int direction,
                                dReal radius, dReal length);
  INLINE void set_cylinder(dReal density, int direction,
                           dReal radius, dReal length);
  INLINE void set_cylinder_total(dReal total_mass, int direction,
                                 dReal radius, dReal length);
  INLINE void set_box(dReal density,
                      dReal lx, dReal ly, dReal lz);
  INLINE void set_box_total(dReal total_mass,
                            dReal lx, dReal ly, dReal lz);
  INLINE void adjust(dReal newmass);
  INLINE void translate(dReal x, dReal y, dReal z);
  INLINE void rotate(const dMatrix3 R);
  INLINE void add(OdeMass &other);

  INLINE dReal get_magnitude() const;
  INLINE LPoint3f get_center() const;
  INLINE LMatrix3f get_inertia() const;

  virtual void write(ostream &out = cout, unsigned int indent=0) const;

public:
  dMass* get_mass_ptr();

private:
  void operator = (const OdeMass &copy);
  dMass _mass;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "OdeMass",
		  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "odeMass.I"

#endif
