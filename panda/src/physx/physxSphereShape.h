// Filename: physxSphereShape.h
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

#ifndef PHYSXSPHERESHAPE_H
#define PHYSXSPHERESHAPE_H

#include "pandabase.h"
#include "luse.h"

#include "physxShape.h"
#include "physx_includes.h"

class PhysxSphereShapeDesc;

////////////////////////////////////////////////////////////////////
//       Class : PhysxSphereShape
// Description : A sphere shaped collision detection primitive. 
//               Each shape is owned by an actor that it is attached
//               to.
//
//               An instance can be created by calling the
//               createShape() method of the PhysxActor object that
//               should own it, with a PhysxSphereShapeDesc object
//               as the parameter, or by adding the shape descriptor
//               into the PhysxActorDesc class before creating the
//               actor.
//
//               The shape is deleted by calling release() on the
//               shape itself.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxSphereShape : public PhysxShape {

PUBLISHED:
  INLINE PhysxSphereShape();
  INLINE ~PhysxSphereShape();

  void save_to_desc(PhysxSphereShapeDesc &shapeDesc) const;

  void set_radius(float radius);

  float get_radius() const;

////////////////////////////////////////////////////////////////////
public:
  INLINE NxShape *ptr() const { return (NxShape *)_ptr; };

  void link(NxShape *shapePtr);
  void unlink();

private:
  NxSphereShape *_ptr;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PhysxShape::init_type();
    register_type(_type_handle, "PhysxSphereShape", 
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

#include "physxSphereShape.I"

#endif // PHYSXSPHERESHAPE_H
