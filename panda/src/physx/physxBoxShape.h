// Filename: physxBoxShape.h
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

#ifndef PHYSXBOXSHAPE_H
#define PHYSXBOXSHAPE_H

#ifdef HAVE_PHYSX

#include "pandabase.h"

#include "physx_enumerations.h"
#include "physxManager.h"
#include "luse.h"

#include "physxShape.h"

class PhysxBox;
class PhysxBoxShapeDesc;

#include "NxPhysics.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxBoxShape
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxBoxShape : public PhysxShape {
PUBLISHED:

  LVecBase3f get_dimensions() const;
  void get_world_obb(PhysxBox & obb) const;
  void save_to_desc(PhysxBoxShapeDesc & desc) const;
  void set_dimensions(const LVecBase3f & vec);


public:
  NxBoxShape *nBoxShape;

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

#endif // HAVE_PHYSX

#endif // PHYSXBOXSHAPE_H
