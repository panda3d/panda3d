// Filename: angularVectorForce.h
// Created by:  charles (09Aug00)
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

#ifndef ANGULARVECTORFORCE_H
#define ANGULARVECTORFORCE_H

#include "angularForce.h"

////////////////////////////////////////////////////////////////////
//       Class : AngularVectorForce
// Description : a simple directed torque force, the angular
//               equivalent of simple vector force.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS AngularVectorForce : public AngularForce {
PUBLISHED:
  AngularVectorForce(const LRotationf& quat);
  AngularVectorForce(float h, float p, float r);
  AngularVectorForce(const AngularVectorForce &copy);
  virtual ~AngularVectorForce();

  INLINE void set_quat(const LRotationf& quat);
  INLINE void set_hpr(float h, float p, float r);
  INLINE LRotationf get_local_quat() const;
  
  virtual void output(ostream &out) const;
  virtual void write(ostream &out, unsigned int indent=0) const;

private:
  LRotationf _fvec;

  virtual AngularForce *make_copy() const;
  virtual LRotationf get_child_quat(const PhysicsObject *po);

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
