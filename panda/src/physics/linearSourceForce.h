// Filename: linearSourceForce.h
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

#ifndef LINEARSOURCEFORCE_H
#define LINEARSOURCEFORCE_H

#include "linearDistanceForce.h"

////////////////////////////////////////////////////////////////////
//       Class : LinearSourceForce
// Description : Repellant force.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS LinearSourceForce : public LinearDistanceForce {
PUBLISHED:
  LinearSourceForce(const LPoint3f& p, FalloffType f, float r, float a = 1.0f,
              bool mass = true);
  LinearSourceForce(void);
  LinearSourceForce(const LinearSourceForce &copy);
  virtual ~LinearSourceForce(void);
  
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
    register_type(_type_handle, "LinearSourceForce",
                  LinearDistanceForce::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif // LINEARSOURCEFORCE_H
