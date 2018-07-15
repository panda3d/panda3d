/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxActorDesc.h
 * @author enn0x
 * @date 2009-09-05
 */

#ifndef PHYSXACTORDESC_H
#define PHYSXACTORDESC_H

#include "pandabase.h"
#include "luse.h"

#include "physx_includes.h"

class PhysxBodyDesc;
class PhysxShapeDesc;

/**
 * Descriptor for PhysxActor.
 */
class EXPCL_PANDAPHYSX PhysxActorDesc {

PUBLISHED:
  INLINE PhysxActorDesc();
  INLINE ~PhysxActorDesc();

  INLINE void set_to_default();
  INLINE bool is_valid() const;

  void add_shape(PhysxShapeDesc &desc);

  void set_name(const char *name);
  void set_density(float density);
  void set_global_pos(const LPoint3f &pos);
  void set_global_mat(const LMatrix4f &mat);
  void set_global_hpr(float h, float p, float r);
  void set_body(PhysxBodyDesc &desc);

  const char *get_name() const;
  float get_density() const;
  LPoint3f get_global_pos() const;
  LMatrix4f get_global_mat() const;
  PhysxBodyDesc get_body() const;

public:
  NxActorDesc _desc;

private:
  std::string _name;
};

#include "physxActorDesc.I"

#endif // PHYSXACTORDESC_H
