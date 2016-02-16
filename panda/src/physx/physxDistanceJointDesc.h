/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxDistanceJointDesc.h
 * @author enn0x
 * @date 2009-09-28
 */

#ifndef PHYSXDISTANCEJOINTDESC_H
#define PHYSXDISTANCEJOINTDESC_H

#include "pandabase.h"

#include "physxJointDesc.h"
#include "physx_includes.h"

class PhysxSpringDesc;

/**
 * Descriptor class for distance joint.  See PhysxDistanceJoint.
 */
class EXPCL_PANDAPHYSX PhysxDistanceJointDesc : public PhysxJointDesc {

PUBLISHED:
  INLINE PhysxDistanceJointDesc();
  INLINE ~PhysxDistanceJointDesc();

  INLINE void set_to_default();
  INLINE bool is_valid() const;

  void set_max_distance(float distance);
  void set_min_distance(float distance);
  void set_spring(const PhysxSpringDesc &spring);
  void set_flag(PhysxDistanceJointFlag flag, bool value);

  float get_max_distance() const;
  float get_min_distance() const;
  bool get_flag(PhysxDistanceJointFlag flag) const;
  PhysxSpringDesc get_spring() const;

public:
  NxJointDesc *ptr() const { return (NxJointDesc *)&_desc; };
  NxDistanceJointDesc _desc;
};

#include "physxDistanceJointDesc.I"

#endif // PHYSXDISTANCEJOINTDESC_H
