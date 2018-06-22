/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file angularVectorForce.h
 * @author charles
 * @date 2000-08-09
 */

#ifndef ANGULARVECTORFORCE_H
#define ANGULARVECTORFORCE_H

#include "angularForce.h"

/**
 * a simple directed torque force, the angular equivalent of simple vector
 * force.
 */
class EXPCL_PANDA_PHYSICS AngularVectorForce : public AngularForce {
PUBLISHED:
  explicit AngularVectorForce(const LRotation& quat);
  explicit AngularVectorForce(PN_stdfloat h, PN_stdfloat p, PN_stdfloat r);
  AngularVectorForce(const AngularVectorForce &copy);
  virtual ~AngularVectorForce();

  INLINE void set_quat(const LRotation& quat);
  INLINE void set_hpr(PN_stdfloat h, PN_stdfloat p, PN_stdfloat r);
  INLINE LRotation get_local_quat() const;

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent=0) const;

private:
  LRotation _fvec;

  virtual AngularForce *make_copy() const;
  virtual LRotation get_child_quat(const PhysicsObject *po);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    AngularForce::init_type();
    register_type(_type_handle, "AngularVectorForce",
                  AngularForce::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "angularVectorForce.I"

#endif // ANGULARVECTORFORCE_H
