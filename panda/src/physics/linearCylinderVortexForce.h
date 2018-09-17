/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file linearCylinderVortexForce.h
 * @author charles
 * @date 2000-07-24
 */

#ifndef LINEARCYLINDERVORTEXFORCE_H
#define LINEARCYLINDERVORTEXFORCE_H

#include "linearForce.h"

/**
 * Defines a cylinder inside of which all forces are tangential to the theta
 * of the particle wrt the z-axis in local coord.  space.  This happens by
 * assigning the force a node by which the cylinder is transformed.  Be
 * warned- this will suck anything that it can reach directly into orbit and
 * will NOT let go.
 */
class EXPCL_PANDA_PHYSICS LinearCylinderVortexForce : public LinearForce {
PUBLISHED:
  explicit LinearCylinderVortexForce(PN_stdfloat radius = 1.0f,
                                     PN_stdfloat length = 0.0f,
                                     PN_stdfloat coef = 1.0f,
                                     PN_stdfloat a = 1.0f,
                                     bool md = false);
  LinearCylinderVortexForce(const LinearCylinderVortexForce &copy);
  virtual ~LinearCylinderVortexForce();

  INLINE void set_coef(PN_stdfloat coef);
  INLINE PN_stdfloat get_coef() const;

  INLINE void set_radius(PN_stdfloat radius);
  INLINE PN_stdfloat get_radius() const;

  INLINE void set_length(PN_stdfloat length);
  INLINE PN_stdfloat get_length() const;

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent=0) const;

private:
  PN_stdfloat _radius;
  PN_stdfloat _length;
  PN_stdfloat _coef;

  virtual LinearForce *make_copy();
  virtual LVector3 get_child_vector(const PhysicsObject *po);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    LinearForce::init_type();
    register_type(_type_handle, "LinearCylinderVortexForce",
                  LinearForce::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "linearCylinderVortexForce.I"

#endif // LINEARCYLINDERVORTEXFORCE_H
