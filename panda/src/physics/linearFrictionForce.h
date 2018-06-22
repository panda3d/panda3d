/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file linearFrictionForce.h
 * @author charles
 * @date 2000-06-23
 */

#ifndef LINEARFRICTIONFORCE_H
#define LINEARFRICTIONFORCE_H

#include "linearForce.h"

/**
 * Friction-based drag force
 */
class EXPCL_PANDA_PHYSICS LinearFrictionForce : public LinearForce {
PUBLISHED:
  explicit LinearFrictionForce(PN_stdfloat coef = 1.0f, PN_stdfloat a = 1.0f, bool m = false);
  LinearFrictionForce(const LinearFrictionForce &copy);
  virtual ~LinearFrictionForce();

  INLINE void set_coef(PN_stdfloat coef);
  INLINE PN_stdfloat get_coef() const;

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent=0) const;

private:
  PN_stdfloat _coef;

  virtual LinearForce *make_copy();
  virtual LVector3 get_child_vector(const PhysicsObject *);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    LinearForce::init_type();
    register_type(_type_handle, "LinearFrictionForce",
                  LinearForce::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "linearFrictionForce.I"

#endif // LINEARFRICTIONFORCE_H
