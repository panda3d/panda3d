/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file linearSourceForce.h
 * @author charles
 * @date 2000-06-21
 */

#ifndef LINEARSOURCEFORCE_H
#define LINEARSOURCEFORCE_H

#include "linearDistanceForce.h"

/**
 * Repellant force.
 */
class EXPCL_PANDA_PHYSICS LinearSourceForce : public LinearDistanceForce {
PUBLISHED:
  explicit LinearSourceForce(const LPoint3& p, FalloffType f, PN_stdfloat r,
                             PN_stdfloat a = 1.0f, bool mass = true);
  LinearSourceForce();
  LinearSourceForce(const LinearSourceForce &copy);
  virtual ~LinearSourceForce();

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
    register_type(_type_handle, "LinearSourceForce",
                  LinearDistanceForce::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif // LINEARSOURCEFORCE_H
