// Filename: linearUserDefinedForce.h
// Created by:  charles (31Jul00)
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

#ifndef LINEARUSERDEFINEDFORCE_H
#define LINEARUSERDEFINEDFORCE_H

#include "linearForce.h"

////////////////////////////////////////////////////////////////////
//       Class : LinearUserDefinedForce
// Description : a programmable force that takes an evaluator fn.
//
//        NOTE : AS OF Interrogate => Squeak, this class does NOT
//               get FFI'd due to the function pointer bug, and is
//               currently NOT getting interrogated.  Change this
//               in the makefile when the time is right or this class
//               becomes needed...
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS LinearUserDefinedForce : public LinearForce {
PUBLISHED:
  LinearUserDefinedForce(LVector3 (*proc)(const PhysicsObject *) = NULL,
                         PN_stdfloat a = 1.0f,
                         bool md = false);
  LinearUserDefinedForce(const LinearUserDefinedForce &copy);
  virtual ~LinearUserDefinedForce();

  INLINE void set_proc(LVector3 (*proc)(const PhysicsObject *));
  
  virtual void output(ostream &out) const;
  virtual void write(ostream &out, unsigned int indent=0) const;

private:
  LVector3 (*_proc)(const PhysicsObject *po);

  virtual LVector3 get_child_vector(const PhysicsObject *po);
  virtual LinearForce *make_copy();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    LinearForce::init_type();
    register_type(_type_handle, "LinearUserDefinedForce",
                  LinearForce::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "linearUserDefinedForce.I"

#endif // LINEARUSERDEFINEDFORCE_H
