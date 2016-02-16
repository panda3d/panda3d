/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxWheelDesc.h
 * @author enn0x
 * @date 2010-03-23
 */

#ifndef PHYSXWHEELDESC_H
#define PHYSXWHEELDESC_H

#include "pandabase.h"
#include "luse.h"

#include "physx_includes.h"

/**

 */
class EXPCL_PANDAPHYSX PhysxWheelDesc : public ReferenceCount {

PUBLISHED:
  INLINE PhysxWheelDesc();
  INLINE ~PhysxWheelDesc();

  INLINE void set_to_default();
  INLINE bool is_valid() const;

public:
};

#include "physxWheelDesc.I"

#endif // PHYSXWHEELDESC_H
