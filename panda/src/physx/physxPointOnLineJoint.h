// Filename: physxPointOnLineJoint.h
// Created by:  enn0x (02Oct09)
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

#ifndef PHYSXPOINTONLINEJOINT_H
#define PHYSXPOINTONLINEJOINT_H

#include "pandabase.h"

#include "physxJoint.h"
#include "physx_includes.h"

class PhysxPointOnLineJointDesc;

////////////////////////////////////////////////////////////////////
//       Class : PhysxPointOnLineJoint
// Description : A point on line joint constrains a point on one
//               body to only move along a line attached to another
//               body. 
//               The starting point of the joint is defined as the
//               anchor point. The line through this point is
//               specified by its direction (axis) vector.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxPointOnLineJoint : public PhysxJoint {

PUBLISHED:
  INLINE PhysxPointOnLineJoint();
  INLINE ~PhysxPointOnLineJoint();

  void save_to_desc(PhysxPointOnLineJointDesc &jointDesc) const;
  void load_from_desc(const PhysxPointOnLineJointDesc &jointDesc);

////////////////////////////////////////////////////////////////////
public:
  INLINE NxJoint *ptr() const { return (NxJoint *)_ptr; };

  void link(NxJoint *jointPtr);
  void unlink();

private:
  NxPointOnLineJoint *_ptr;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PhysxJoint::init_type();
    register_type(_type_handle, "PhysxPointOnLineJoint", 
                  PhysxJoint::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

#include "physxPointOnLineJoint.I"

#endif // PHYSXPOINTONLINEJOINT_H
