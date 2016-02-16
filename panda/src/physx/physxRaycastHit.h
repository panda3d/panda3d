/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxRaycastHit.h
 * @author enn0x
 * @date 2009-10-21
 */

#ifndef PHYSXRAYCASTHIT_H
#define PHYSXRAYCASTHIT_H

#include "pandabase.h"
#include "luse.h"

#include "config_physx.h"

class PhysxShape;

/**
 * This structure captures results for a single raycast query.  See PhysxScene
 * for raycasting methods.
 */
class EXPCL_PANDAPHYSX PhysxRaycastHit {

PUBLISHED:
  INLINE PhysxRaycastHit(const NxRaycastHit hit);
  INLINE ~PhysxRaycastHit();

  bool is_empty() const;

  PhysxShape *get_shape() const;
  LPoint3f get_impact_pos() const;
  LVector3f get_impact_normal() const;
  float get_distance() const;

private:
  NxRaycastHit _hit;
};

#include "physxRaycastHit.I"

#endif // PHYSXRAYCASTHIT_H
