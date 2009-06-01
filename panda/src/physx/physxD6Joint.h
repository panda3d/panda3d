// Filename: physxD6Joint.h
// Created by:  pratt (Jun 16, 2006)
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

#ifdef HAVE_PHYSX

#include "pandabase.h"

#include "physx_enumerations.h"
#include "physxManager.h"
#include "luse.h"

#include "physxJoint.h"

class PhysxD6JointDesc;

#include "NxPhysics.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxD6Joint
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxD6Joint : public PhysxJoint {
PUBLISHED:

  void load_from_desc(const PhysxD6JointDesc & desc);
  void save_to_desc(PhysxD6JointDesc & desc);
  void set_drive_angular_velocity(const LVecBase3f & ang_vel);
  void set_drive_linear_velocity(const LVecBase3f & lin_vel);
  void set_drive_orientation(const LQuaternionf & orientation);
  void set_drive_position(const LVecBase3f & position);


public:
  NxD6Joint *nD6Joint;

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

#endif // HAVE_PHYSX

#endif // PHYSXD6JOINT_H
