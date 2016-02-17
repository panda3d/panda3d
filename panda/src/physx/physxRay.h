/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxRay.h
 * @author enn0x
 * @date 2009-10-21
 */

#ifndef PHYSXRAY_H
#define PHYSXRAY_H

#include "pandabase.h"
#include "luse.h"

#include "config_physx.h"

/**
 * Represents an ray as an origin and direction.  The ray will be infinite if no
 * length is given.
 */
class EXPCL_PANDAPHYSX PhysxRay {

PUBLISHED:
  INLINE PhysxRay();
  INLINE ~PhysxRay();

  void set_length(float length);
  void set_origin(const LPoint3f &origin);
  void set_direction(const LVector3f &direction);

  float get_length() const;
  LPoint3f get_origin() const;
  LVector3f get_direction() const;

public:
  NxRay _ray;
  NxReal _length;
};

#include "physxRay.I"

#endif // PHYSRAY_H
