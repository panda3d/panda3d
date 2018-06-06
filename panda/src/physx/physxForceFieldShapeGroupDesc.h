/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxForceFieldShapeGroupDesc.h
 * @author enn0x
 * @date 2009-11-11
 */

#ifndef PHYSXFORCEFIELDSHAPEGROUPDESC_H
#define PHYSXFORCEFIELDSHAPEGROUPDESC_H

#include "pandabase.h"

#include "physxEnums.h"
#include "physx_includes.h"

class PhysxForceFieldShapeDesc;

/**
 *
 */
class EXPCL_PANDAPHYSX PhysxForceFieldShapeGroupDesc : public PhysxEnums {

PUBLISHED:
  INLINE PhysxForceFieldShapeGroupDesc();
  INLINE ~PhysxForceFieldShapeGroupDesc();

  INLINE void set_to_default();
  INLINE bool is_valid() const;

  void add_shape(PhysxForceFieldShapeDesc &desc);

  void set_name(const char *name);
  void set_flag(PhysxForceFieldShapeGroupFlag flag, bool value);

  const char *get_name() const;
  bool get_flag(PhysxForceFieldShapeGroupFlag flag) const;

public:
  NxForceFieldShapeGroupDesc _desc;

private:
  std::string _name;
};

#include "physxForceFieldShapeGroupDesc.I"

#endif // PHYSXFORCEFIELDSHAPEGROUPDESC_H
