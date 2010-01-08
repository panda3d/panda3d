// Filename: physxSphericalJoint.h
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

#ifndef PHYSXSPHERICALJOINT_H
#define PHYSXSPHERICALJOINT_H

#include "pandabase.h"

#include "physxJoint.h"
#include "physx_includes.h"

class PhysxSphericalJointDesc;

////////////////////////////////////////////////////////////////////
//       Class : PhysxSphericalJoint
// Description : A sphere joint constrains two points on two bodies
//               to coincide. This point, specified in world space
//               (this guarantees that the points coincide to start
//               with) is the only parameter that has to be
//               specified.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxSphericalJoint : public PhysxJoint {

PUBLISHED:
  INLINE PhysxSphericalJoint();
  INLINE ~PhysxSphericalJoint();

  void save_to_desc(PhysxSphericalJointDesc &jointDesc) const;
  void load_from_desc(const PhysxSphericalJointDesc &jointDesc);

  void set_flag(PhysxSphericalJointFlag flag, bool value);
  void set_projection_mode(PhysxProjectionMode mode);

  bool get_flag(PhysxSphericalJointFlag flag) const;
  PhysxProjectionMode get_projection_mode() const;

////////////////////////////////////////////////////////////////////
public:
  INLINE NxJoint *ptr() const { return (NxJoint *)_ptr; };

  void link(NxJoint *jointPtr);
  void unlink();

private:
  NxSphericalJoint *_ptr;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PhysxJoint::init_type();
    register_type(_type_handle, "PhysxSphericalJoint", 
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

#include "physxSphericalJoint.I"

#endif // PHYSXSPHERICALJOINT_H
