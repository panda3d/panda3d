/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxPrismaticJointDesc.h
 * @author enn0x
 * @date 2009-09-28
 */

#ifndef PHYSXPRISMATICJOINTDESC_H
#define PHYSXPRISMATICJOINTDESC_H

#include "pandabase.h"

#include "physxJointDesc.h"
#include "physx_includes.h"

/**
 * Descriptor class for prismatic joint.  See PhysxPrismaticJoint.
 */
class EXPCL_PANDAPHYSX PhysxPrismaticJointDesc : public PhysxJointDesc {

PUBLISHED:
  INLINE PhysxPrismaticJointDesc();
  INLINE ~PhysxPrismaticJointDesc();

  INLINE void set_to_default();
  INLINE bool is_valid() const;

public:
  NxJointDesc *ptr() const { return (NxJointDesc *)&_desc; };
  NxPrismaticJointDesc _desc;
};

#include "physxPrismaticJointDesc.I"

#endif // PHYSXPRISMATICJOINTDESC_H
