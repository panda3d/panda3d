// Filename: linearJitterForce.h
// Created by:  charles (13Jun00)
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
  LinearJitterForce(float a = 1.0f, bool m = false);
  LinearJitterForce(const LinearJitterForce &copy);
  virtual ~LinearJitterForce(void);
  
  virtual void output(ostream &out, unsigned int indent=0) const;

private:
  virtual LVector3f get_child_vector(const PhysicsObject *po);
  virtual LinearForce *make_copy(void);

public:
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    LinearRandomForce::init_type();
    register_type(_type_handle, "LinearJitterForce",
                  LinearRandomForce::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif // LINEARJITTERFORCE_H
