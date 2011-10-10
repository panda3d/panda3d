// Filename: linearJitterForce.h
// Created by:  charles (13Jun00)
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

#ifndef LINEARJITTERFORCE_H
#define LINEARJITTERFORCE_H

#include "linearRandomForce.h"

////////////////////////////////////////////////////////////////////
//       Class : LinearJitterForce
// Description : Completely random noise force vector.  Not
//               repeatable, reliable, or predictable.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS LinearJitterForce : public LinearRandomForce {
PUBLISHED:
  LinearJitterForce(PN_stdfloat a = 1.0f, bool m = false);
  LinearJitterForce(const LinearJitterForce &copy);
  virtual ~LinearJitterForce();
  
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
    LinearRandomForce::init_type();
    register_type(_type_handle, "LinearJitterForce",
                  LinearRandomForce::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif // LINEARJITTERFORCE_H
