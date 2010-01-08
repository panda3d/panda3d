// Filename: physxCylindricalJoint.h
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

#ifndef PHYSXCYLINDRICALJOINT_H
#define PHYSXCYLINDRICALJOINT_H

#include "pandabase.h"

#include "physxJoint.h"
#include "physx_includes.h"

class PhysxCylindricalJointDesc;

////////////////////////////////////////////////////////////////////
//       Class : PhysxCylindricalJoint
// Description : Cylindrical Joints permit relative translational
//               movement between two bodies along an axis, and also
//               relative rotation along the axis. 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxCylindricalJoint : public PhysxJoint {

PUBLISHED:
  INLINE PhysxCylindricalJoint();
  INLINE ~PhysxCylindricalJoint();

  void save_to_desc(PhysxCylindricalJointDesc &jointDesc) const;
  void load_from_desc(const PhysxCylindricalJointDesc &jointDesc);

////////////////////////////////////////////////////////////////////
public:
  INLINE NxJoint *ptr() const { return (NxJoint *)_ptr; };

  void link(NxJoint *jointPtr);
  void unlink();

private:
  NxCylindricalJoint *_ptr;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PhysxJoint::init_type();
    register_type(_type_handle, "PhysxCylindricalJoint", 
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

#include "physxCylindricalJoint.I"

#endif // PHYSXCYLINDRICALJOINT_H
