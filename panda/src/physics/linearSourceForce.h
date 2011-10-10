// Filename: linearSourceForce.h
// Created by:  charles (21Jun00)
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

#ifndef LINEARSOURCEFORCE_H
#define LINEARSOURCEFORCE_H

#include "linearDistanceForce.h"

////////////////////////////////////////////////////////////////////
//       Class : LinearSourceForce
// Description : Repellant force.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS LinearSourceForce : public LinearDistanceForce {
PUBLISHED:
  LinearSourceForce(const LPoint3& p, FalloffType f, PN_stdfloat r, PN_stdfloat a = 1.0f,
              bool mass = true);
  LinearSourceForce();
  LinearSourceForce(const LinearSourceForce &copy);
  virtual ~LinearSourceForce();
  
  virtual void output(ostream &out) const;
  virtual void write(ostream &out, unsigned int indent=0) const;

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
