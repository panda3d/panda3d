// Filename: physxD6Joint.h
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

#ifndef PHYSXD6JOINT_H
#define PHYSXD6JOINT_H

#include "pandabase.h"

#include "physxJoint.h"
#include "physx_includes.h"

class PhysxD6JointDesc;

////////////////////////////////////////////////////////////////////
//       Class : PhysxD6Joint
// Description : A D6 joint is a general constraint between two
//               actors.  It allows the user to individually define
//               the linear and rotational degrees of freedom.  It
//               also allows the user to configure the joint with
//               limits and driven degrees of freedom as they wish.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxD6Joint : public PhysxJoint {

PUBLISHED:
  INLINE PhysxD6Joint();
  INLINE ~PhysxD6Joint();

  void save_to_desc(PhysxD6JointDesc &jointDesc) const;
  void load_from_desc(const PhysxD6JointDesc &jointDesc);

  void set_drive_angular_velocity(const LVector3f &v);
  void set_drive_linear_velocity(const LVector3f &v);
  void set_drive_orientation(const LQuaternionf &quat);
  void set_drive_position(const LPoint3f &pos);

////////////////////////////////////////////////////////////////////
public:
  INLINE NxJoint *ptr() const { return (NxJoint *)_ptr; };

  void link(NxJoint *jointPtr);
  void unlink();

private:
  NxD6Joint *_ptr;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PhysxJoint::init_type();
    register_type(_type_handle, "PhysxD6Joint", 
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

#include "physxD6Joint.I"

#endif // PHYSXD6JOINT_H
