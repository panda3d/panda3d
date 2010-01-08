// Filename: physxPointInPlaneJoint.h
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

#ifndef PHYSXPOINTINPLANEJOINT_H
#define PHYSXPOINTINPLANEJOINT_H

#include "pandabase.h"

#include "physxJoint.h"
#include "physx_includes.h"

class PhysxPointInPlaneJointDesc;

////////////////////////////////////////////////////////////////////
//       Class : PhysxPointInPlaneJoint
// Description : A point in plane joint constrains a point on one
//               body to only move inside a plane attached to another
//               body. 
//               The starting point of the point is defined as the
//               anchor point. The plane through this point is
//               specified by its normal which is the joint axis
//               vector.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxPointInPlaneJoint : public PhysxJoint {

PUBLISHED:
  INLINE PhysxPointInPlaneJoint();
  INLINE ~PhysxPointInPlaneJoint();

  void save_to_desc(PhysxPointInPlaneJointDesc &jointDesc) const;
  void load_from_desc(const PhysxPointInPlaneJointDesc &jointDesc);

////////////////////////////////////////////////////////////////////
public:
  INLINE NxJoint *ptr() const { return (NxJoint *)_ptr; };

  void link(NxJoint *jointPtr);
  void unlink();

private:
  NxPointInPlaneJoint *_ptr;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PhysxJoint::init_type();
    register_type(_type_handle, "PhysxPointInPlaneJoint", 
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

#include "physxPointInPlaneJoint.I"

#endif // PHYSXPOINTINPLANEJOINT_H
