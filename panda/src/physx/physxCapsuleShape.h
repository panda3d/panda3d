// Filename: physxCapsuleShape.h
// Created by:  enn0x (16Sep09)
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

#ifndef PHYSXCAPSULESHAPE_H
#define PHYSXCAPSULESHAPE_H

#include "pandabase.h"
#include "luse.h"

#include "physxShape.h"
#include "physx_includes.h"

class PhysxCapsuleShapeDesc;

////////////////////////////////////////////////////////////////////
//       Class : PhysxCapsuleShape
// Description : A capsule shaped collision detection primitive,
//               also known as a line swept sphere. 'radius' is the
//               radius of the capsule's hemispherical ends and its
//               trunk. 'height' is the distance between the two
//               hemispherical ends of the capsule. The height is
//               along the capsule's Y axis. Each shape is owned by
//               an actor that it is attached to.
//
//               An instance can be created by calling the
//               createShape() method of the PhysxActor object that
//               should own it, with a PhysxCapsuleShapeDesc object
//               as the parameter, or by adding the shape descriptor
//               into the PhysxActorDesc class before creating the
//               actor.
//
//               The shape is deleted by calling release() on the
//               shape itself.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxCapsuleShape : public PhysxShape {

PUBLISHED:
  INLINE PhysxCapsuleShape();
  INLINE ~PhysxCapsuleShape();

  void save_to_desc(PhysxCapsuleShapeDesc &shapeDesc) const;

  void set_radius(float radius);
  void set_height(float height);

  float get_radius() const;
  float get_height() const;

////////////////////////////////////////////////////////////////////
public:
  INLINE NxShape *ptr() const { return (NxShape *)_ptr; };

  void link(NxShape *shapePtr);
  void unlink();

private:
  NxCapsuleShape *_ptr;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PhysxShape::init_type();
    register_type(_type_handle, "PhysxCapsuleShape", 
                  PhysxShape::get_class_type());
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

#include "physxCapsuleShape.I"

#endif // PHYSXCAPSULESHAPE_H
