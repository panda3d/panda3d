// Filename: linearUserDefinedForce.h
// Created by:  charles (31Jul00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
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
class LinearUserDefinedForce : public LinearForce {
PUBLISHED:
  LinearUserDefinedForce(LVector3f (*proc)(const PhysicsObject *) = NULL,
                   float a = 1.0f,
                   bool md = false);
  LinearUserDefinedForce(const LinearUserDefinedForce &copy);
  virtual ~LinearUserDefinedForce(void);

  INLINE void set_proc(LVector3f (*proc)(const PhysicsObject *));
  
  virtual void output(ostream &out, unsigned int indent=0) const;

private:
  LVector3f (*_proc)(const PhysicsObject *po);

  virtual LVector3f get_child_vector(const PhysicsObject *po);
  virtual LinearForce *make_copy(void);

public:
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    LinearForce::init_type();
    register_type(_type_handle, "LinearUserDefinedForce",
                  LinearForce::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "linearUserDefinedForce.I"

#endif // LINEARUSERDEFINEDFORCE_H
