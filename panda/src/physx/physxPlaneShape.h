// Filename: physxPlaneShape.h
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

#ifndef PHYSXPLANESHAPE_H
#define PHYSXPLANESHAPE_H

#include "pandabase.h"
#include "luse.h"

#include "physxShape.h"
#include "physx_includes.h"

class PhysxPlaneShapeDesc;

////////////////////////////////////////////////////////////////////
//       Class : PhysxPlaneShape
// Description : A plane collision detection primitive. By default
//               it is configured to be the y == 0 plane. You can
//               then set a normal and a d to specify an arbitrary
//               plane. d is the distance of the plane from the
//               origin along the normal, assuming the normal is
//               normalized. Thus the plane equation is:
//               normal.x * X + normal.y * Y + normal.z * Z = d
//
//               Note: the plane does not represent an infinitely
//               thin object, but rather a completely solid
//               negative half space (all points p for which
//               normal.dot(p) - d < 0 are inside the solid region.)
//
//               Each shape is owned by an actor that it is attached
//               to.
//
//               An instance can be created by calling the
//               createShape() method of the PhysxActor object that
//               should own it, with a PhysxPlaneShapeDesc object as
//               the parameter, or by adding the shape descriptor
//               into the PhysxActorDesc class before creating the
//               actor.
//
//               The shape is deleted by calling release() on the
//               shape itself.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxPlaneShape : public PhysxShape {

PUBLISHED:
  INLINE PhysxPlaneShape();
  INLINE ~PhysxPlaneShape();

  void save_to_desc(PhysxPlaneShapeDesc &shapeDesc) const;

  void set_plane(const LVector3f &normal, float d);

////////////////////////////////////////////////////////////////////
public:
  INLINE NxShape *ptr() const { return (NxShape *)_ptr; };

  void link(NxShape *shapePtr);
  void unlink();

private:
  NxPlaneShape *_ptr;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PhysxShape::init_type();
    register_type(_type_handle, "PhysxPlaneShape", 
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

#include "physxPlaneShape.I"

#endif // PHYSXPLANESHAPE_H
