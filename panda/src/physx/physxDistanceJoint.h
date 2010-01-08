// Filename: physxDistanceJoint.h
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

#ifndef PHYSXDISTANCEJOINT_H
#define PHYSXDISTANCEJOINT_H

#include "pandabase.h"

#include "physxJoint.h"
#include "physx_includes.h"

class PhysxDistanceJointDesc;

////////////////////////////////////////////////////////////////////
//       Class : PhysxDistanceJoint
// Description : A distance joint maintains a certain distance
//               between two points on two actors. 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxDistanceJoint : public PhysxJoint {

PUBLISHED:
  INLINE PhysxDistanceJoint();
  INLINE ~PhysxDistanceJoint();

  void save_to_desc(PhysxDistanceJointDesc &jointDesc) const;
  void load_from_desc(const PhysxDistanceJointDesc &jointDesc);

////////////////////////////////////////////////////////////////////
public:
  INLINE NxJoint *ptr() const { return (NxJoint *)_ptr; };

  void link(NxJoint *jointPtr);
  void unlink();

private:
  NxDistanceJoint *_ptr;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PhysxJoint::init_type();
    register_type(_type_handle, "PhysxDistanceJoint", 
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

#include "physxDistanceJoint.I"

#endif // PHYSXDISTANCEJOINT_H
