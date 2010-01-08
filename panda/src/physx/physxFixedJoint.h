// Filename: physxFixedJoint.h
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

#ifndef PHYSXFIXEDJOINT_H
#define PHYSXFIXEDJOINT_H

#include "pandabase.h"

#include "physxJoint.h"
#include "physx_includes.h"

class PhysxFixedJointDesc;

////////////////////////////////////////////////////////////////////
//       Class : PhysxFixedJoint
// Description : A fixed joint permits no relative movement between
//               two bodies. ie the bodies are glued together. 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxFixedJoint : public PhysxJoint {

PUBLISHED:
  INLINE PhysxFixedJoint();
  INLINE ~PhysxFixedJoint();

  void save_to_desc(PhysxFixedJointDesc &jointDesc) const;
  void load_from_desc(const PhysxFixedJointDesc &jointDesc);

////////////////////////////////////////////////////////////////////
public:
  INLINE NxJoint *ptr() const { return (NxJoint *)_ptr; };

  void link(NxJoint *jointPtr);
  void unlink();

private:
  NxFixedJoint *_ptr;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PhysxJoint::init_type();
    register_type(_type_handle, "PhysxFixedJoint", 
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

#include "physxFixedJoint.I"

#endif // PHYSXFIXEDJOINT_H
