// Filename: physxSphereShapeDesc.h
// Created by:  enn0x (11Sep09)
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

#ifndef PHYSXSPHERESHAPEDESC_H
#define PHYSXSPHERESHAPEDESC_H

#include "pandabase.h"

#include "physxShapeDesc.h"
#include "physx_includes.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxSphereShapeDesc
// Description : Descriptor class for PhysxSphereShape.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxSphereShapeDesc : public PhysxShapeDesc {

PUBLISHED:
  INLINE PhysxSphereShapeDesc();
  INLINE ~PhysxSphereShapeDesc();

  INLINE void set_to_default();
  INLINE bool is_valid() const;

  void set_radius(float radius);

  float get_radius() const;

public:
  NxShapeDesc *ptr() const { return (NxShapeDesc *)&_desc; };
  NxSphereShapeDesc _desc;
};

#include "physxSphereShapeDesc.I"

#endif // PHYSXSPHERESHAPEDESC_H
