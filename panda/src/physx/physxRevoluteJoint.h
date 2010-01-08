// Filename: physxRevoluteJoint.h
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

#ifndef PHYSXREVOLUTEJOINT_H
#define PHYSXREVOLUTEJOINT_H

#include "pandabase.h"

#include "physxJoint.h"
#include "physx_includes.h"

class PhysxRevoluteJointDesc;
class PhysxSpringDesc;
class PhysxMotorDesc;
class PhysxJointLimitDesc;

////////////////////////////////////////////////////////////////////
//       Class : PhysxRevoluteJoint
// Description : A joint which behaves in a similar way to a hinge
//               or axel. A hinge joint removes all but a single
//               rotational degree of freedom from two objects. The
//               axis along which the two bodies may rotate is
//               specified with a point and a direction vector.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxRevoluteJoint : public PhysxJoint {

PUBLISHED:
  INLINE PhysxRevoluteJoint();
  INLINE ~PhysxRevoluteJoint();

  void save_to_desc(PhysxRevoluteJointDesc &jointDesc) const;
  void load_from_desc(const PhysxRevoluteJointDesc &jointDesc);

  void set_spring(const PhysxSpringDesc &spring);
  void set_motor(const PhysxMotorDesc &motor);
  void set_limits(const PhysxJointLimitDesc &low, const PhysxJointLimitDesc &high);
  void set_flag(PhysxRevoluteJointFlag flag, bool value);
  void set_projection_mode(PhysxProjectionMode mode);

  float get_angle() const;
  float get_velocity() const;
  bool get_flag(PhysxRevoluteJointFlag flag) const;
  PhysxProjectionMode get_projection_mode() const;
  PhysxMotorDesc get_motor() const;
  PhysxSpringDesc get_spring() const;

////////////////////////////////////////////////////////////////////
public:
  INLINE NxJoint *ptr() const { return (NxJoint *)_ptr; };

  void link(NxJoint *jointPtr);
  void unlink();

private:
  NxRevoluteJoint *_ptr;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PhysxJoint::init_type();
    register_type(_type_handle, "PhysxRevoluteJoint", 
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

#include "physxRevoluteJoint.I"

#endif // PHYSXREVOLUTEJOINT_H
