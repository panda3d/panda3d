// Filename: physxConvexForceFieldShape.h
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

#ifndef PHYSXCONVEXFORCEFIELDSHAPE_H
#define PHYSXCONVEXFORCEFIELDSHAPE_H

#include "pandabase.h"

#include "physxForceFieldShape.h"
#include "physx_includes.h"

class PhysxConvexForceFieldShapeDesc;

////////////////////////////////////////////////////////////////////
//       Class : PhysxConvexForceFieldShape
// Description : A convex shaped region used to define force field.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxConvexForceFieldShape : public PhysxForceFieldShape {

PUBLISHED:
  INLINE PhysxConvexForceFieldShape();
  INLINE ~PhysxConvexForceFieldShape();

  void save_to_desc(PhysxConvexForceFieldShapeDesc &shapeDesc) const;

////////////////////////////////////////////////////////////////////
public:
  INLINE NxForceFieldShape *ptr() const { return (NxForceFieldShape *)_ptr; };

  void link(NxForceFieldShape *shapePtr);
  void unlink();

private:
  NxConvexForceFieldShape *_ptr;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PhysxForceFieldShape::init_type();
    register_type(_type_handle, "PhysxConvexForceFieldShape", 
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

#include "physxConvexForceFieldShape.I"

#endif // PHYSXCONVEXFORCEFIELDSHAPE_H
