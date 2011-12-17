// Filename: physxBoxControllerDesc.h
// Created by:  enn0x (22Sep09)
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

#ifndef PHYSXBOXCONTROLLERDESC_H
#define PHYSXBOXCONTROLLERDESC_H

#include "pandabase.h"
#include "luse.h"

#include "physxControllerDesc.h"
#include "physx_includes.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxBoxControllerDesc
// Description : Descriptor class for PhysxBoxController.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxBoxControllerDesc : public PhysxControllerDesc {

PUBLISHED:
  INLINE PhysxBoxControllerDesc();
  INLINE ~PhysxBoxControllerDesc();

  INLINE void set_to_default();
  INLINE bool is_valid() const;

  void set_extents(const LVector3f &extents);

  LVector3f get_extents() const;

public:
  NxControllerDesc *ptr() const { return (NxControllerDesc *)&_desc; };
  NxBoxControllerDesc _desc;
};

#include "physxBoxControllerDesc.I"

#endif // PHYSXBOXCONTROLLERDESC_H
