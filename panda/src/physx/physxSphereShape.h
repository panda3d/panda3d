// Filename: physxSphereShape.h
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

#ifndef PHYSXSPHERESHAPE_H
#define PHYSXSPHERESHAPE_H

#ifdef HAVE_PHYSX

#include "pandabase.h"

#include "physx_enumerations.h"
#include "physxManager.h"
#include "luse.h"

#include "physxShape.h"

class PhysxSphere;
class PhysxSphereShapeDesc;

#include "NxPhysics.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxSphereShape
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxSphereShape : public PhysxShape {
PUBLISHED:

  float get_radius() const;
  void get_world_sphere(PhysxSphere & world_sphere) const;
  void save_to_desc(PhysxSphereShapeDesc & desc) const;
  void set_radius(float radius);


public:
  NxSphereShape *nSphereShape;

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

#endif // HAVE_PHYSX

#endif // PHYSXSPHERESHAPE_H
