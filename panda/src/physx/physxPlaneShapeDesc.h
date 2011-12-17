// Filename: physxPlaneShapeDesc.h
// Created by:  enn0x (08Sep09)
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

#ifndef PHYSXPLANESHAPEDESC_H
#define PHYSXPLANESHAPEDESC_H

#include "pandabase.h"
#include "physxShapeDesc.h"

#include "luse.h"

#include "physx_includes.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxPlaneShapeDesc
// Description : Descriptor class for PhysxPlaneShape.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxPlaneShapeDesc : public PhysxShapeDesc {

PUBLISHED:
  INLINE PhysxPlaneShapeDesc();
  INLINE ~PhysxPlaneShapeDesc();

  INLINE void set_to_default();
  INLINE bool is_valid() const;

  void set_plane(const LVector3f &normal, float d);

public:
  NxShapeDesc *ptr() const { return (NxShapeDesc *)&_desc; };
  NxPlaneShapeDesc _desc;
};

#include "physxPlaneShapeDesc.I"

#endif // PHYSXPLANESHAPEDESC_H
