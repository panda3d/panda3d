// Filename: physxCylindricalJointDesc.h
// Created by:  enn0x (28Sep09)
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

#ifndef PHYSXCYLINDRICALJOINTDESC_H
#define PHYSXCYLINDRICALJOINTDESC_H

#include "pandabase.h"

#include "physxJointDesc.h"
#include "physx_includes.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxCylindricalJointDesc
// Description : Descriptor class for sliding joint. See
//               PhysxCylindricalJoint.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxCylindricalJointDesc : public PhysxJointDesc {

PUBLISHED:
  INLINE PhysxCylindricalJointDesc();
  INLINE ~PhysxCylindricalJointDesc();

  INLINE void set_to_default();
  INLINE bool is_valid() const;

public:
  NxJointDesc *ptr() const { return (NxJointDesc *)&_desc; };
  NxCylindricalJointDesc _desc;
};

#include "physxCylindricalJointDesc.I"

#endif // PHYSXCYLINDRICALJOINTDESC_H
