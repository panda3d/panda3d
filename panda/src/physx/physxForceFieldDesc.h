/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxForceFieldDesc.h
 * @author enn0x
 * @date 2009-11-06
 */

#ifndef PHYSXFORCEFIELDDESC_H
#define PHYSXFORCEFIELDDESC_H

#include "pandabase.h"
#include "luse.h"

#include "physxEnums.h"
#include "physx_includes.h"

class PhysxActor;
class PhysxForceFieldShapeDesc;
class PhysxForceFieldShapeGroup;

/**
 * Descriptor class for force fields.
 */
class EXPCL_PANDAPHYSX PhysxForceFieldDesc : public PhysxEnums {

PUBLISHED:
  INLINE PhysxForceFieldDesc();
  INLINE ~PhysxForceFieldDesc();

  INLINE void set_to_default();
  INLINE bool is_valid() const;

  void set_name(const char *name);
  void set_pos(const LPoint3f &pos);
  void set_mat(const LMatrix4f &mat);
  void set_hpr(float h, float p, float r);
  void set_coordinates(PhysxForceFieldCoordinates coordinates);
  void set_actor(PhysxActor *actor);

  void set_kernel_constant(const LVector3f &constant);
  void set_kernel_position_target(const LPoint3f &target);
  void set_kernel_position_multiplier(const LMatrix3f &multiplier);
  void set_kernel_velocity_target(const LVector3f &target);
  void set_kernel_velocity_multiplier(const LMatrix3f &multiplier);
  void set_kernel_torus_radius(float radius);
  void set_kernel_falloff_linear(const LVector3f &falloff);
  void set_kernel_falloff_quadratic(const LVector3f &falloff);
  void set_kernel_noise(const LVector3f &noise);

  void add_include_group_shape(PhysxForceFieldShapeDesc &shapeDesc);
  void add_shape_group(PhysxForceFieldShapeGroup *group);

public:
  void create_kernel(NxScene *scenePtr);

  NxForceFieldDesc _desc;
  NxForceFieldLinearKernelDesc _kernel;

private:
  std::string _name;
};

#include "physxForceFieldDesc.I"

#endif // PHYSXFORCEFIELDDESC_H
