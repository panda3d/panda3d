// Filename: physxBoxShape.h
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

#ifndef PHYSXBOXSHAPE_H
#define PHYSXBOXSHAPE_H

#include "pandabase.h"
#include "luse.h"

#include "physxShape.h"
#include "physx_includes.h"

class PhysxBoxShapeDesc;

////////////////////////////////////////////////////////////////////
//       Class : PhysxBoxShape
// Description : A box shaped collision detection primitive. Each
//               shape is owned by the actor which it is attached
//               to.
//
//               An instance can be created by calling the
//               createShape() method of the PhysxActor object that
//               will own it, with a PhysxBoxShapeDesc object as the
//               parameter, or by adding the shape descriptor to the
//               PhysxActorDesc class before creating the actor.
//
//               The shape is deleted by calling release() on the
//               shape itself.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxBoxShape : public PhysxShape {

PUBLISHED:
  INLINE PhysxBoxShape();
  INLINE ~PhysxBoxShape();

  void save_to_desc(PhysxBoxShapeDesc &shapeDesc) const;

  void set_dimensions(const LVector3f &dimensions);
  LVector3f get_dimensions() const;

////////////////////////////////////////////////////////////////////
public:
  INLINE NxShape *ptr() const { return (NxShape *)_ptr; };

  void link(NxShape *shapePtr);
  void unlink();

private:
  NxBoxShape *_ptr;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PhysxShape::init_type();
    register_type(_type_handle, "PhysxBoxShape", 
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

#include "physxBoxShape.I"

#endif // PHYSXBOXSHAPE_H
