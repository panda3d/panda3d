// Filename: odeMass.I
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

INLINE int OdeMass::
check() {
  return dMassCheck(&_mass);
}

INLINE void OdeMass::
set_zero() {
  _mass.setZero();
}

INLINE void OdeMass::
set_parameters(dReal themass,
               dReal cgx, dReal cgy, dReal cgz,
               dReal I11, dReal I22, dReal I33,
               dReal I12, dReal I13, dReal I23) {
  _mass.setParameters(themass,
                      cgx, cgy, cgz,
                      I11, I22, I33,
                      I12, I13, I23);
}

INLINE void OdeMass::
set_parameters(dReal themass,
               const LVecBase3f &center,
               const LMatrix3f &i) {
  set_parameters(themass,
                 center[0], center[1], center[2], 
                 i(0, 0), i(1, 1), i(2, 2),
                 i(0, 1), i(0, 2), i(1, 2));
}

INLINE void OdeMass::
set_sphere(dReal density, dReal radius) {
  _mass.setSphere(density, radius);
}

INLINE void OdeMass::
set_sphere_total(dReal total_mass, dReal radius) {
  dMassSetSphereTotal(&_mass, total_mass, radius);
}

INLINE void OdeMass::
set_capsule(dReal density, int direction,
            dReal radius, dReal length) {
  _mass.setCapsule(density, direction,
                    radius, length);
}

INLINE void OdeMass::
set_capsule_total(dReal total_mass, int direction,
                  dReal radius, dReal length) {
  dMassSetCapsuleTotal(&_mass, 
                       total_mass, direction,
                       radius, length);
}

INLINE void OdeMass::
set_cylinder(dReal density, int direction,
             dReal radius, dReal length) {
  dMassSetCylinder(&_mass, 
                   density,direction,
                   radius,length);
}

INLINE void OdeMass::
set_cylinder_total(dReal total_mass, int direction,
                   dReal radius, dReal length) {
  dMassSetCylinderTotal(&_mass, total_mass, direction,
                        radius, length);
}

INLINE void OdeMass::
set_box(dReal density,
        dReal lx, dReal ly, dReal lz) {
  _mass.setBox(density,
               lx, ly, lz);
}

INLINE void OdeMass::
set_box(dReal density,
        const LVecBase3f &size) {
  _mass.setBox(density,
               size[0], size[1], size[2]);
}

INLINE void OdeMass::
set_box_total(dReal total_mass,
              const LVecBase3f &size) {
  dMassSetBoxTotal(&_mass,
                   total_mass,
                   size[0], size[1], size[2]);
}

INLINE void OdeMass::
set_box_total(dReal total_mass,
              dReal lx, dReal ly, dReal lz) {
  dMassSetBoxTotal(&_mass,
                   total_mass,
                   lx, ly, lz);
}

INLINE void OdeMass::
adjust(dReal newmass) {
  _mass.adjust(newmass);
}

INLINE void OdeMass::
translate(dReal x, dReal y, dReal z) {
  _mass.translate(x, y, z);
}

INLINE void OdeMass::
translate(const LVecBase3f &pos) {
  translate(pos[0], pos[1], pos[2]);
}

INLINE void OdeMass::
rotate(const LMatrix3f &r) {
  dMatrix3 rot = { r(0, 0), r(0, 1), r(0, 2), 0,
                   r(1, 0), r(1, 1), r(1, 2), 0,
                   r(2, 0), r(2, 1), r(2, 2), 0 };
  _mass.rotate(rot);
}

INLINE void OdeMass::
add(OdeMass &other) {
  _mass.add(other.get_mass_ptr());
}

INLINE dReal OdeMass::
get_magnitude() const {
  return _mass.mass;
}

INLINE LPoint3f OdeMass::
get_center() const {
  return LPoint3f(_mass.c[0], _mass.c[1], _mass.c[2]);
}

INLINE LMatrix3f OdeMass::
get_inertial_tensor() const {
  return LMatrix3f(_mass.I[0], _mass.I[1], _mass.I[2] ,
                   _mass.I[4], _mass.I[5], _mass.I[6] ,
                   _mass.I[8], _mass.I[9], _mass.I[10]);
}
