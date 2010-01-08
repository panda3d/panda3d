// Filename: physxCapsuleControllerDesc.h
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

#ifndef PHYSXCAPSULECONTROLLERDESC_H
#define PHYSXCAPSULECONTROLLERDESC_H

#include "pandabase.h"

#include "physxControllerDesc.h"
#include "physx_includes.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxCapsuleControllerDesc
// Description : Descriptor class for PhysxCapsuleController.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxCapsuleControllerDesc : public PhysxControllerDesc {

PUBLISHED:
  INLINE PhysxCapsuleControllerDesc();
  INLINE ~PhysxCapsuleControllerDesc();

  INLINE void set_to_default();
  INLINE bool is_valid() const;

  void set_radius(float radius);
  void set_height(float height);

  float get_radius() const;
  float get_height() const;

public:
  NxControllerDesc *ptr() const { return (NxControllerDesc *)&_desc; };
  NxCapsuleControllerDesc _desc;
};

#include "physxCapsuleControllerDesc.I"

#endif // PHYSXCAPSULECONTROLLERDESC_H
