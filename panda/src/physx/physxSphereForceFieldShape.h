// Filename: physxSphereForceFieldShape.h
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

#ifndef PHYSXSPHEREFORCEFIELDSHAPE_H
#define PHYSXSPHEREFORCEFIELDSHAPE_H

#include "pandabase.h"
#include "luse.h"

#include "physxForceFieldShape.h"
#include "physx_includes.h"

class PhysxSphereForceFieldShapeDesc;

////////////////////////////////////////////////////////////////////
//       Class : PhysxSphereForceFieldShape
// Description : A spherical force field shape.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxSphereForceFieldShape : public PhysxForceFieldShape {

PUBLISHED:
  INLINE PhysxSphereForceFieldShape();
  INLINE ~PhysxSphereForceFieldShape();

  void save_to_desc(PhysxSphereForceFieldShapeDesc &shapeDesc) const;

  void set_radius(float radius);

  float get_radius() const;

////////////////////////////////////////////////////////////////////
public:
  INLINE NxForceFieldShape *ptr() const { return (NxForceFieldShape *)_ptr; };

  void link(NxForceFieldShape *shapePtr);
  void unlink();

private:
  NxSphereForceFieldShape *_ptr;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PhysxForceFieldShape::init_type();
    register_type(_type_handle, "PhysxSphereForceFieldShape", 
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

#include "physxSphereForceFieldShape.I"

#endif // PHYSXSPHEREFORCEFIELDSHAPE_H
