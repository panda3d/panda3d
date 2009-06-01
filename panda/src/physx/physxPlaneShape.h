// Filename: physxPlaneShape.h
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

#ifndef PHYSXPLANESHAPE_H
#define PHYSXPLANESHAPE_H

#ifdef HAVE_PHYSX

#include "pandabase.h"

#include "physx_enumerations.h"
#include "physxManager.h"
#include "luse.h"

#include "physxShape.h"

class PhysxPlane;
class PhysxPlaneShapeDesc;

#include "NxPhysics.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxPlaneShape
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxPlaneShape : public PhysxShape {
PUBLISHED:

  PhysxPlane get_plane() const;
  void save_to_desc(PhysxPlaneShapeDesc & desc) const;
  void set_plane(const LVecBase3f & normal, float d);


public:
  NxPlaneShape *nPlaneShape;

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

#endif // HAVE_PHYSX

#endif // PHYSXPLANESHAPE_H
