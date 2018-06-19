/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file angularForce.h
 * @author charles
 * @date 2000-08-08
 */

#ifndef ANGULARFORCE_H
#define ANGULARFORCE_H

#include "baseForce.h"

/**
 * pure virtual parent of all quat-based forces.
 */
class EXPCL_PANDA_PHYSICS AngularForce : public BaseForce {
PUBLISHED:
  virtual ~AngularForce();

  virtual AngularForce *make_copy() const = 0;
  LRotation get_quat(const PhysicsObject *po);
  virtual bool is_linear() const;

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent=0) const;

protected:
  AngularForce();
  AngularForce(const AngularForce &copy);

private:
  virtual LRotation get_child_quat(const PhysicsObject *po) = 0;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    BaseForce::init_type();
    register_type(_type_handle, "AngularForce",
                  BaseForce::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif // ANGULARFORCE_H
