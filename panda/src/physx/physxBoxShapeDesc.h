/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxBoxShapeDesc.h
 * @author enn0x
 * @date 2009-09-08
 */

#ifndef PHYSXBOXSHAPEDESC_H
#define PHYSXBOXSHAPEDESC_H

#include "pandabase.h"
#include "luse.h"

#include "physxShapeDesc.h"
#include "physx_includes.h"

/**
 * Descriptor class for PhysxBoxShape.
 */
class EXPCL_PANDAPHYSX PhysxBoxShapeDesc : public PhysxShapeDesc {

PUBLISHED:
  INLINE PhysxBoxShapeDesc();
  INLINE ~PhysxBoxShapeDesc();

  INLINE void set_to_default();
  INLINE bool is_valid() const;

  void set_dimensions(const LVector3f &dimensions);

  LVector3f get_dimensions() const;

public:
  NxShapeDesc *ptr() const { return (NxShapeDesc *)&_desc; };
  NxBoxShapeDesc _desc;
};

#include "physxBoxShapeDesc.I"

#endif // PHYSXBOXSHAPEDESC_H
