// Filename: physxBoxForceFieldShapeDesc.h
// Created by:  enn0x (06Nov09)
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

#ifndef PHYSXBOXFORCEFIELDSHAPEDESC_H
#define PHYSXBOXFORCEFIELDSHAPEDESC_H

#include "pandabase.h"
#include "luse.h"

#include "physxForceFieldShapeDesc.h"
#include "physx_includes.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxBoxForceFieldShapeDesc
// Description : Descriptor for a box force field shape.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxBoxForceFieldShapeDesc : public PhysxForceFieldShapeDesc {

PUBLISHED:
  INLINE PhysxBoxForceFieldShapeDesc();
  INLINE ~PhysxBoxForceFieldShapeDesc();

  INLINE void set_to_default();
  INLINE bool is_valid() const;

  void set_dimensions(const LVector3f &dimensions);

  LVector3f get_dimensions() const;

public:
  NxForceFieldShapeDesc *ptr() const { return (NxForceFieldShapeDesc *)&_desc; };
  NxBoxForceFieldShapeDesc _desc;
};

#include "physxBoxForceFieldShapeDesc.I"

#endif // PHYSXBOXFORCEFIELDSHAPEDESC_H
