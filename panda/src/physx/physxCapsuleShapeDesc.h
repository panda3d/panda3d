/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxCapsuleShapeDesc.h
 * @author enn0x
 * @date 2009-09-11
 */

#ifndef PHYSXCAPSULESHAPEDESC_H
#define PHYSXCAPSULESHAPEDESC_H

#include "pandabase.h"

#include "physxShapeDesc.h"
#include "physx_includes.h"

/**
 * Descriptor class for PhysxCapsuleShape.
 */
class EXPCL_PANDAPHYSX PhysxCapsuleShapeDesc : public PhysxShapeDesc {

PUBLISHED:
  INLINE PhysxCapsuleShapeDesc();
  INLINE ~PhysxCapsuleShapeDesc();

  INLINE void set_to_default();
  INLINE bool is_valid() const;

  void set_radius(float radius);
  void set_height(float height);

  float get_radius() const;
  float get_height() const;

public:
  NxShapeDesc *ptr() const { return (NxShapeDesc *)&_desc; };
  NxCapsuleShapeDesc _desc;
};

#include "physxCapsuleShapeDesc.I"

#endif // PHYSXCAPSULESHAPEDESC_H
