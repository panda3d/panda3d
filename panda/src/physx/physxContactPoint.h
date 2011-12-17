// Filename: physxContactPoint.h
// Created by:  enn0x (20Dec09)
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

#ifndef PHYSXCONTACTPOINT_H
#define PHYSXCONTACTPOINT_H

#include "pandabase.h"
#include "luse.h"
#include "typedObject.h"

#include "config_physx.h"

class PhysxShape;

////////////////////////////////////////////////////////////////////
//       Class : PhysxContactPoint
// Description : A helper structure for iterating over contact
//               streams reported by PhysxContactPair.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxContactPoint : public TypedObject {

PUBLISHED:
  INLINE PhysxContactPoint();
  INLINE ~PhysxContactPoint();

  LPoint3f get_point() const;
  LVector3f get_normal() const;
  float get_normal_force() const;
  float get_separation() const;
  unsigned int get_feature_index0() const;
  unsigned int get_feature_index1() const;

public:
  static PhysxContactPoint empty();

  void set(NxContactStreamIterator it);

private:
  NxVec3 _point;
  NxVec3 _normal;
  NxReal _normal_force;
  NxReal _separation;
  NxU32 _feature_index0;
  NxU32 _feature_index1;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "PhysxContactPoint", 
                  TypedReferenceCount::get_class_type());
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

#include "physxContactPoint.I"

#endif // PHYSXCONTACTPOINT_H
