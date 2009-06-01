// Filename: physxBoxShapeDesc.h
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

#ifndef PHYSXBOXSHAPEDESC_H
#define PHYSXBOXSHAPEDESC_H

#ifdef HAVE_PHYSX

#include "pandabase.h"

#include "physx_enumerations.h"
#include "physxManager.h"
#include "luse.h"

#include "physxShapeDesc.h"

#include "NxPhysics.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxBoxShapeDesc
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxBoxShapeDesc : public PhysxShapeDesc {
PUBLISHED:
  PhysxBoxShapeDesc();

  INLINE bool is_valid() const;
  INLINE void set_to_default();

  LVecBase3f get_dimensions() const;

  void set_dimensions( LVecBase3f value );

public:
  NxBoxShapeDesc nBoxShapeDesc;
};

#include "physxBoxShapeDesc.I"

#endif // HAVE_PHYSX

#endif // PHYSXBOXSHAPEDESC_H
