/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxForceFieldShapeDesc.h
 * @author enn0x
 * @date 2009-11-06
 */

#ifndef PHYSXFORCEFIELDSHAPEDESC_H
#define PHYSXFORCEFIELDSHAPEDESC_H

#include "pandabase.h"
#include "luse.h"

#include "physx_includes.h"

/**
 * Abstract base class for descriptors for force field shapes descriptors.
 */
class EXPCL_PANDAPHYSX PhysxForceFieldShapeDesc {

PUBLISHED:
  virtual void set_to_default() = 0;
  virtual bool is_valid() const = 0;

  void set_name(const char *name);
  void set_pos(const LPoint3f &pos);
  void set_mat(const LMatrix4f &mat);
  void set_hpr(float h, float p, float r);

  const char *get_name() const;
  LPoint3f get_pos() const;
  LMatrix4f get_mat() const;

public:
  virtual NxForceFieldShapeDesc *ptr() const = 0;

private:
  std::string _name;

protected:
  INLINE PhysxForceFieldShapeDesc();
  INLINE ~PhysxForceFieldShapeDesc();
};

#include "physxForceFieldShapeDesc.I"

#endif // PHYSXFORCEFIELDSHAPEDESC_H
