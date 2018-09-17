/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxJointDesc.h
 * @author enn0x
 * @date 2009-09-28
 */

#ifndef PHYSXJOINTDESC_H
#define PHYSXJOINTDESC_H

#include "pandabase.h"
#include "luse.h"

#include "physxEnums.h"
#include "physx_includes.h"

class PhysxActor;

/**
 * Abstract base class for joint descriptors.
 */
class EXPCL_PANDAPHYSX PhysxJointDesc : public PhysxEnums {

PUBLISHED:
  virtual void set_to_default() = 0;
  virtual bool is_valid() const = 0;

  void set_name(const char *name);
  void set_joint_flag(PhysxJointFlag flag, bool value);
  void set_max_force(float force);
  void set_max_torque(float torque);
  void set_solver_extrapolation_factor(float factor);
  void set_global_axis(const LVector3f &axis);
  void set_global_anchor(const LPoint3f &anchor);
  void set_local_normal(unsigned int idx, const LVector3f &normal);
  void set_local_axis(unsigned int idx, const LVector3f &axis);
  void set_local_anchor(unsigned int idx, const LPoint3f &anchor);
  void set_actor(unsigned int idx, const PhysxActor &actor);

  const char *get_name() const;
  bool get_joint_flag(PhysxJointFlag flag) const;
  float get_max_force() const;
  float get_max_torque() const;
  float get_solver_extrapolation_factor() const;
  LVector3f get_local_normal(unsigned int idx) const;
  LVector3f get_local_axis(unsigned int idx) const;
  LPoint3f get_local_anchor(unsigned int idx) const;
  PhysxActor *get_actor(unsigned int idx) const;

public:
  virtual NxJointDesc *ptr() const = 0;

private:
  std::string _name;

protected:
  INLINE PhysxJointDesc();
  INLINE ~PhysxJointDesc();
};

#include "physxJointDesc.I"

#endif // PHYSXJOINTDESC_H
