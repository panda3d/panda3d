// Filename: physxTriangleMeshShapeDesc.h
// Created by:  enn0x (14Oct09)
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

#ifndef PHYSXTRIANGLEMESHSHAPEDESC_H
#define PHYSXTRIANGLEMESHSHAPEDESC_H

#include "pandabase.h"

#include "physxShapeDesc.h"
#include "physx_includes.h"

class PhysxTriangleMesh;

////////////////////////////////////////////////////////////////////
//       Class : PhysxTriangleMeshShapeDesc
// Description : Descriptor class for PhysxTriangleMeshShape.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxTriangleMeshShapeDesc : public PhysxShapeDesc {

PUBLISHED:
  INLINE PhysxTriangleMeshShapeDesc();
  INLINE ~PhysxTriangleMeshShapeDesc();

  INLINE void set_to_default();
  INLINE bool is_valid() const;

  void set_mesh(PhysxTriangleMesh *mesh);

public:
  NxShapeDesc *ptr() const { return (NxShapeDesc *)&_desc; };
  NxTriangleMeshShapeDesc _desc;
};

#include "physxTriangleMeshShapeDesc.I"

#endif // PHYSXTRIANGLEMESHSHAPEDESC_H
