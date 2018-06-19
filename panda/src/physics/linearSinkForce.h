/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file linearSinkForce.h
 * @author charles
 * @date 2000-06-21
 */

#ifndef LINEARSINKFORCE_H
#define LINEARSINKFORCE_H

#include "linearDistanceForce.h"

/**
 * Attractor force.  Think black hole.
 */
class EXPCL_PANDA_PHYSICS LinearSinkForce : public LinearDistanceForce {
PUBLISHED:
  explicit LinearSinkForce(const LPoint3& p, FalloffType f, PN_stdfloat r,
                           PN_stdfloat a = 1.0f, bool m = true);
  LinearSinkForce();
  LinearSinkForce(const LinearSinkForce &copy);
  virtual ~LinearSinkForce();

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent=0) const;

private:
  virtual LVector3 get_child_vector(const PhysicsObject *po);
  virtual LinearForce *make_copy();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    LinearDistanceForce::init_type();
    register_type(_type_handle, "LinearSinkForce",
                  LinearDistanceForce::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif // LINEARSINKFORCE_H
