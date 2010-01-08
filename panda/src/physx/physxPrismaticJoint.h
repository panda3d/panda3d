// Filename: physxPrismaticJoint.h
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

#ifndef PHYSXPRISMATICJOINT_H
#define PHYSXPRISMATICJOINT_H

#include "pandabase.h"

#include "physxJoint.h"
#include "physx_includes.h"

class PhysxPrismaticJointDesc;

////////////////////////////////////////////////////////////////////
//       Class : PhysxPrismaticJoint
// Description : A prismatic joint permits relative translational
//               movement between two bodies along an axis, but no
//               relative rotational movement. 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxPrismaticJoint : public PhysxJoint {

PUBLISHED:
  INLINE PhysxPrismaticJoint();
  INLINE ~PhysxPrismaticJoint();

  void save_to_desc(PhysxPrismaticJointDesc &jointDesc) const;
  void load_from_desc(const PhysxPrismaticJointDesc &jointDesc);

////////////////////////////////////////////////////////////////////
public:
  INLINE NxJoint *ptr() const { return (NxJoint *)_ptr; };

  void link(NxJoint *jointPtr);
  void unlink();

private:
  NxPrismaticJoint *_ptr;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PhysxJoint::init_type();
    register_type(_type_handle, "PhysxPrismaticJoint", 
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

#include "physxPrismaticJoint.I"

#endif // PHYSXPRISMATICJOINT_H
