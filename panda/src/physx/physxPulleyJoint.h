// Filename: physxPulleyJoint.h
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

#ifndef PHYSXPULLEYJOINT_H
#define PHYSXPULLEYJOINT_H

#include "pandabase.h"

#include "physxJoint.h"
#include "physx_includes.h"

class PhysxMotorDesc;
class PhysxPulleyJointDesc;

////////////////////////////////////////////////////////////////////
//       Class : PhysxPulleyJoint
// Description : A pulley joint simulates a rope between two
//               objects passing over two pulleys.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxPulleyJoint : public PhysxJoint {

PUBLISHED:
  INLINE PhysxPulleyJoint();
  INLINE ~PhysxPulleyJoint();

  void save_to_desc(PhysxPulleyJointDesc &jointDesc) const;
  void load_from_desc(const PhysxPulleyJointDesc &jointDesc);

  void set_motor(const PhysxMotorDesc &motor);
  void set_flag(PhysxPulleyJointFlag flag, bool value);

  bool get_flag(PhysxPulleyJointFlag flag) const;
  PhysxMotorDesc get_motor() const;

////////////////////////////////////////////////////////////////////
public:
  INLINE NxJoint *ptr() const { return (NxJoint *)_ptr; };

  void link(NxJoint *jointPtr);
  void unlink();

private:
  NxPulleyJoint *_ptr;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PhysxJoint::init_type();
    register_type(_type_handle, "PhysxPulleyJoint", 
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

#include "physxPulleyJoint.I"

#endif // PHYSXPULLEYJOINT_H
