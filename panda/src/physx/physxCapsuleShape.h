// Filename: physxCapsuleShape.h
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

#ifndef PHYSXCAPSULESHAPE_H
#define PHYSXCAPSULESHAPE_H

#ifdef HAVE_PHYSX

#include "pandabase.h"

#include "physx_enumerations.h"
#include "physxManager.h"
#include "luse.h"

#include "physxShape.h"

class PhysxCapsule;
class PhysxCapsuleShapeDesc;

#include "NxPhysics.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxCapsuleShape
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxCapsuleShape : public PhysxShape {
PUBLISHED:

  float get_height() const;
  float get_radius() const;
  void get_world_capsule(PhysxCapsule & world_capsule) const;
  void save_to_desc(PhysxCapsuleShapeDesc & desc) const;
  void set_dimensions(float radius, float height);
  void set_height(float height);
  void set_radius(float radius);


public:
  NxCapsuleShape *nCapsuleShape;

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

#endif // HAVE_PHYSX

#endif // PHYSXCAPSULESHAPE_H
