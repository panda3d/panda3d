// Filename: angularVectorForce.h
// Created by:  charles (09Aug00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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
  AngularVectorForce(const LVector3f& vec);
  AngularVectorForce(float x = 0.0f, float y = 0.0f, float z = 0.0f);
  AngularVectorForce(const AngularVectorForce &copy);
  virtual ~AngularVectorForce();

  INLINE void set_vector(const LVector3f& v);
  INLINE void set_vector(float x, float y, float z);
  INLINE LVector3f get_local_vector() const;
  
  virtual void output(ostream &out) const;
  virtual void write(ostream &out, unsigned int indent=0) const;

private:
  LVector3f _fvec;

  virtual AngularForce *make_copy() const;
  virtual LVector3f get_child_vector(const PhysicsObject *po);

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
