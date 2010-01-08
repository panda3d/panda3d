// Filename: physxConvexForceFieldShapeDesc.h
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

#ifndef PHYSXCONVEXFORCEFIELDSHAPEDESC_H
#define PHYSXCONVEXFORCEFIELDSHAPEDESC_H

#include "pandabase.h"

#include "physxForceFieldShapeDesc.h"
#include "physx_includes.h"

class PhysxConvexMesh;

////////////////////////////////////////////////////////////////////
//       Class : PhysxConvexForceFieldShapeDesc
// Description : A descriptor for a convex force field shape.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxConvexForceFieldShapeDesc : public PhysxForceFieldShapeDesc {

PUBLISHED:
  INLINE PhysxConvexForceFieldShapeDesc();
  INLINE ~PhysxConvexForceFieldShapeDesc();

  INLINE void set_to_default();
  INLINE bool is_valid() const;

  void set_mesh(PhysxConvexMesh *mesh);

public:
  NxForceFieldShapeDesc *ptr() const { return (NxForceFieldShapeDesc *)&_desc; };
  NxConvexForceFieldShapeDesc _desc;
};

#include "physxConvexForceFieldShapeDesc.I"

#endif // PHYSXCONVEXFORCEFIELDSHAPEDESC_H
