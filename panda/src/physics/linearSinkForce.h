// Filename: linearSinkForce.h
// Created by:  charles (21Jun00)
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

#ifndef LINEARSINKFORCE_H
#define LINEARSINKFORCE_H

#include "linearDistanceForce.h"

////////////////////////////////////////////////////////////////////
//       Class : LinearSinkForce
// Description : Attractor force.  Think black hole.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS LinearSinkForce : public LinearDistanceForce {
PUBLISHED:
  LinearSinkForce(const LPoint3f& p, FalloffType f, float r, float a = 1.0f,
            bool m = true);
  LinearSinkForce(void);
  LinearSinkForce(const LinearSinkForce &copy);
  virtual ~LinearSinkForce(void);
  
  virtual void output(ostream &out, unsigned int indent=0) const;

private:
  virtual LVector3f get_child_vector(const PhysicsObject *po);
  virtual LinearForce *make_copy(void);

public:
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    LinearDistanceForce::init_type();
    register_type(_type_handle, "LinearSinkForce",
                  LinearDistanceForce::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif // LINEARSINKFORCE_H
