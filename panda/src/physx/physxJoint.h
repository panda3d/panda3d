// Filename: physxJoint.h
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

#ifndef PHYSXJOINT_H
#define PHYSXJOINT_H

#ifdef HAVE_PHYSX

#include "pandabase.h"

#include "physx_enumerations.h"
#include "physxManager.h"
#include "luse.h"

#include "typedWritableReferenceCount.h"

class PhysxD6Joint;
class PhysxScene;

#include "NxPhysics.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxJoint
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxJoint : public TypedWritableReferenceCount {
PUBLISHED:
  INLINE bool is_valid();

  bool add_limit_plane(const LVecBase3f & normal, const LVecBase3f & point_in_plane, float restitution);
  void get_breakable(float & max_force, float & max_torque);
  LVecBase3f get_global_anchor() const;
  LVecBase3f get_global_axis() const;
  bool get_limit_point(LVecBase3f & world_limit_point);
  const char * get_name() const;
  bool get_next_limit_plane(LVecBase3f & plane_normal, float & plane_d, float * restitution);
  PhysxScene & get_scene() const;
  PhysxJointState get_state();
  bool has_more_limit_planes();
  void * is(PhysxJointType type);
  PhysxD6Joint * is_d6_joint();
  void purge_limit_planes();
  void reset_limit_plane_iterator();
  void set_breakable(float max_force, float max_torque);
  void set_global_anchor(const LVecBase3f & vec);
  void set_global_axis(const LVecBase3f & vec);
  void set_limit_point(const LVecBase3f & point, bool point_is_on_actor2);
  void set_name(const char * name);

  INLINE void * get_app_data() const;

  INLINE void set_app_data( void * value );

public:
  NxJoint *nJoint;

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "PhysxJoint",
                  TypedWritableReferenceCount::get_class_type());
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

  string _name_store;
};

#include "physxJoint.I"

#endif // HAVE_PHYSX

#endif // PHYSXJOINT_H
