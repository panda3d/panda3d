/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file linearRandomForce.h
 * @author charles
 * @date 2000-06-19
 */

#ifndef LINEARRANDOMFORCE_H
#define LINEARRANDOMFORCE_H

#include <stdlib.h>
#include "cmath.h"
#include "mathNumbers.h"
#include "linearForce.h"

/**
 * Pure virtual, parent to noiseForce and jitterForce
 */
class EXPCL_PANDA_PHYSICS LinearRandomForce : public LinearForce {
PUBLISHED:
  virtual ~LinearRandomForce();

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent=0) const;

protected:
  static PN_stdfloat bounded_rand();
  static LVector3 random_unit_vector();

  LinearRandomForce(PN_stdfloat a = 1.0f, bool m = false);
  LinearRandomForce(const LinearRandomForce &copy);

  virtual LVector3 get_child_vector(const PhysicsObject *po) = 0;
  virtual LinearForce *make_copy() = 0;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    LinearForce::init_type();
    register_type(_type_handle, "LinearRandomForce",
                  LinearForce::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "linearRandomForce.I"

#endif // LINEARRANDOMFORCE_H
