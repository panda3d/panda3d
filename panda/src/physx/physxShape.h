// Filename: physxShape.h
// Created by:  pratt (Apr 7, 2006)
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

#ifndef PHYSXSHAPE_H
#define PHYSXSHAPE_H

#ifdef HAVE_PHYSX

#include "pandabase.h"

#include "physx_enumerations.h"
#include "physxManager.h"
#include "luse.h"

#include "typedWritableReferenceCount.h"

class PhysxActorNode;
class PhysxBounds3;
class PhysxBox;
class PhysxBoxShape;
class PhysxCapsule;
class PhysxCapsuleShape;
class PhysxPlaneShape;
class PhysxSphere;
class PhysxSphereShape;

#include "NxPhysics.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxShape
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxShape : public TypedWritableReferenceCount {
PUBLISHED:
  INLINE bool is_valid();

  bool check_overlap_aabb(const PhysxBounds3 & world_bounds) const;
  bool check_overlap_capsule(const PhysxCapsule & world_capsule) const;
  bool check_overlap_obb(const PhysxBox & world_box) const;
  bool check_overlap_sphere(const PhysxSphere & world_sphere) const;
  PhysxActorNode & get_actor() const;
  bool get_flag(PhysxShapeFlag flag) const;
  LMatrix3f get_global_orientation() const;
  LMatrix4f get_global_pose() const;
  LVecBase3f get_global_position() const;
  unsigned short get_group() const;
  LMatrix3f get_local_orientation() const;
  LMatrix4f get_local_pose() const;
  LVecBase3f get_local_position() const;
  unsigned short get_material() const;
  const char * get_name() const;
  float get_skin_width() const;
  void get_world_bounds(PhysxBounds3 & dest) const;
  const PhysxBoxShape * is_box() const;
  const PhysxCapsuleShape * is_capsule() const;
  const PhysxPlaneShape * is_plane() const;
  const PhysxSphereShape * is_sphere() const;
  void set_flag(PhysxShapeFlag flag, bool value);
  void set_global_orientation(const LMatrix3f & mat);
  void set_global_pose(const LMatrix4f & mat);
  void set_global_position(const LVecBase3f & vec);
  void set_group(unsigned short collision_group);
  void set_local_orientation(const LMatrix3f & mat);
  void set_local_pose(const LMatrix4f & mat);
  void set_local_position(const LVecBase3f & vec);
  void set_material(unsigned short mat_index);
  void set_name(const char * name);
  void set_skin_width(float skin_width);

  INLINE void * get_app_data() const;

  INLINE void set_app_data( void * value );

public:
  NxShape *nShape;

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "PhysxShape",
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

#include "physxShape.I"

#endif // HAVE_PHYSX

#endif // PHYSXSHAPE_H
