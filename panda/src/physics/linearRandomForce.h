// Filename: linearRandomForce.h
// Created by:  charles (19Jun00)
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

#ifndef LINEARRANDOMFORCE_H
#define LINEARRANDOMFORCE_H

#include <stdlib.h>
#include <math.h>

#include "linearForce.h"

////////////////////////////////////////////////////////////////////
//       Class : LinearRandomForce
// Description : Pure virtual, parent to noiseForce and jitterForce
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS LinearRandomForce : public LinearForce {
PUBLISHED:
  virtual ~LinearRandomForce(void);
  
  virtual void output(ostream &out, unsigned int indent=0) const;

protected:
  static float bounded_rand(void);
  static LVector3f random_unit_vector(void);

  LinearRandomForce(float a = 1.0f, bool m = false);
  LinearRandomForce(const LinearRandomForce &copy);

  virtual LVector3f get_child_vector(const PhysicsObject *po) = 0;
  virtual LinearForce *make_copy(void) = 0;

public:
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    LinearForce::init_type();
    register_type(_type_handle, "LinearRandomForce",
                  LinearForce::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "linearRandomForce.I"

#endif // LINEARRANDOMFORCE_H
