// Filename: linearVectorForce.h
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

#ifndef LINEARVECTORFORCE_H
#define LINEARVECTORFORCE_H

#include "linearForce.h"

////////////////////////////////////////////////////////////////
//       Class : LinearVectorForce
// Description : Simple directed vector force.  Suitable for
//               gravity, non-turbulent wind, etc...
////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS LinearVectorForce : public LinearForce {
PUBLISHED:
  LinearVectorForce(const LVector3f& vec, float a = 1.0f, bool mass = false);
  LinearVectorForce(const LinearVectorForce &copy);
  LinearVectorForce(float x = 0.0f, float y = 0.0f, float z = 0.0f,
              float a = 1.0f, bool mass = false);
  virtual ~LinearVectorForce();

  INLINE void set_vector(const LVector3f& v);
  INLINE void set_vector(float x, float y, float z);

  INLINE LVector3f get_local_vector() const;
  
  virtual void output(ostream &out) const;
  virtual void write(ostream &out, unsigned int indent=0) const;

public:
  INLINE LinearVectorForce& operator += (const LinearVectorForce &other);

private:
  LVector3f _fvec;

  virtual LinearForce *make_copy();
  virtual LVector3f get_child_vector(const PhysicsObject *po);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    LinearForce::init_type();
    register_type(_type_handle, "LinearVectorForce",
                  LinearForce::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "linearVectorForce.I"

#endif // LINEARVECTORFORCE_H
