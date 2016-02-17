/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxConvexShapeDesc.h
 * @author enn0x
 * @date 2009-10-14
 */

#ifndef PHYSXCONVEXSHAPEDESC_H
#define PHYSXCONVEXSHAPEDESC_H

#include "pandabase.h"

#include "physxShapeDesc.h"
#include "physx_includes.h"

class PhysxConvexMesh;

/**
 * Descriptor class for PhysxConvexShape.
 */
class EXPCL_PANDAPHYSX PhysxConvexShapeDesc : public PhysxShapeDesc {

PUBLISHED:
  INLINE PhysxConvexShapeDesc();
  INLINE ~PhysxConvexShapeDesc();

  INLINE void set_to_default();
  INLINE bool is_valid() const;

  void set_mesh(PhysxConvexMesh *mesh);

public:
  NxShapeDesc *ptr() const { return (NxShapeDesc *)&_desc; };
  NxConvexShapeDesc _desc;
};

#include "physxConvexShapeDesc.I"

#endif // PHYSXCONVEXSHAPEDESC_H
