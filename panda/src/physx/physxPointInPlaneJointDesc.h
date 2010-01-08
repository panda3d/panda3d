// Filename: physxPointInPlaneJointDesc.h
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

#ifndef PHYSXPOINTINPLANEJOINTDESC_H
#define PHYSXPOINTINPLANEJOINTDESC_H

#include "pandabase.h"

#include "physxJointDesc.h"
#include "physx_includes.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxPointInPlaneJointDesc
// Description : Descriptor class for point-in-plane joint. See
//               PhysxPointInPlaneJoint.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxPointInPlaneJointDesc : public PhysxJointDesc {

PUBLISHED:
  INLINE PhysxPointInPlaneJointDesc();
  INLINE ~PhysxPointInPlaneJointDesc();

  INLINE void set_to_default();
  INLINE bool is_valid() const;

public:
  NxJointDesc *ptr() const { return (NxJointDesc *)&_desc; };
  NxPointInPlaneJointDesc _desc;
};

#include "physxPointInPlaneJointDesc.I"

#endif // PHYSXPOINTINPLANEJOINTDESC_H
