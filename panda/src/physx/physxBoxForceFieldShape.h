// Filename: physxBoxForceFieldShape.h
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

#ifndef PHYSXBOXFORCEFIELDSHAPE_H
#define PHYSXBOXFORCEFIELDSHAPE_H

#include "pandabase.h"
#include "luse.h"

#include "physxForceFieldShape.h"
#include "physx_includes.h"

class PhysxBoxForceFieldShapeDesc;

////////////////////////////////////////////////////////////////////
//       Class : PhysxBoxForceFieldShape
// Description : A box shaped region used to define a force field.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxBoxForceFieldShape : public PhysxForceFieldShape {

PUBLISHED:
  INLINE PhysxBoxForceFieldShape();
  INLINE ~PhysxBoxForceFieldShape();

  void save_to_desc(PhysxBoxForceFieldShapeDesc &shapeDesc) const;

  void set_dimensions(const LVector3f &dimensions);
  LVector3f get_dimensions() const;

////////////////////////////////////////////////////////////////////
public:
  INLINE NxForceFieldShape *ptr() const { return (NxForceFieldShape *)_ptr; };

  void link(NxForceFieldShape *shapePtr);
  void unlink();

private:
  NxBoxForceFieldShape *_ptr;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PhysxForceFieldShape::init_type();
    register_type(_type_handle, "PhysxBoxForceFieldShape", 
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

#include "physxBoxForceFieldShape.I"

#endif // PHYSXBOXFORCEFIELDSHAPE_H
