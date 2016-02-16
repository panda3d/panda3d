/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxPointOnLineJointDesc.h
 * @author enn0x
 * @date 2009-09-28
 */

#ifndef PHYSXPOINTONLINEJOINTDESC_H
#define PHYSXPOINTONLINEJOINTDESC_H

#include "pandabase.h"

#include "physxJointDesc.h"
#include "physx_includes.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxPointOnLineJointDesc
// Description : Descriptor class for point-on-line joint.
//               See PhysxPointOnLineJoint.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxPointOnLineJointDesc : public PhysxJointDesc {

PUBLISHED:
  INLINE PhysxPointOnLineJointDesc();
  INLINE ~PhysxPointOnLineJointDesc();

  INLINE void set_to_default();
  INLINE bool is_valid() const;

public:
  NxJointDesc *ptr() const { return (NxJointDesc *)&_desc; };
  NxPointOnLineJointDesc _desc;
};

#include "physxPointOnLineJointDesc.I"

#endif // PHYSXPOINTONLINEJOINTDESC_H
