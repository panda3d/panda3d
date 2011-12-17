// Filename: physxCapsuleForceFieldShape.h
// Created by:  enn0x (15Nov09)
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

#ifndef PHYSXCAPSULEFORCEFIELDSHAPE_H
#define PHYSXCAPSULEFORCEFIELDSHAPE_H

#include "pandabase.h"
#include "luse.h"

#include "physxForceFieldShape.h"
#include "physx_includes.h"

class PhysxCapsuleForceFieldShapeDesc;

////////////////////////////////////////////////////////////////////
//       Class : PhysxCapsuleForceFieldShape
// Description : A capsule shaped region used to define a force
//               field.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxCapsuleForceFieldShape : public PhysxForceFieldShape {

PUBLISHED:
  INLINE PhysxCapsuleForceFieldShape();
  INLINE ~PhysxCapsuleForceFieldShape();

  void save_to_desc(PhysxCapsuleForceFieldShapeDesc &shapeDesc) const;

  void set_radius(float radius);
  void set_height(float height);

  float get_radius() const;
  float get_height() const;

////////////////////////////////////////////////////////////////////
public:
  INLINE NxForceFieldShape *ptr() const { return (NxForceFieldShape *)_ptr; };

  void link(NxForceFieldShape *shapePtr);
  void unlink();

private:
  NxCapsuleForceFieldShape *_ptr;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PhysxForceFieldShape::init_type();
    register_type(_type_handle, "PhysxCapsuleForceFieldShape", 
                  PhysxForceFieldShape::get_class_type());
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

#include "physxCapsuleForceFieldShape.I"

#endif // PHYSXCAPSULEFORCEFIELDSHAPE_H
