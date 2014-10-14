// Filename: physxBodyDesc.h
// Created by:  enn0x (05Sep09)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef PHYSXBODYDESC_H
#define PHYSXBODYDESC_H

#include "pandabase.h"
#include "luse.h"

#include "physxEnums.h"
#include "physx_includes.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxBodyDesc
// Description : Descriptor for the optional rigid body dynamic
//               state of PhysxActor.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxBodyDesc : public PhysxEnums {

PUBLISHED:
  INLINE PhysxBodyDesc();
  INLINE ~PhysxBodyDesc();

  INLINE void set_to_default();
  INLINE bool is_valid() const;

  void set_mass(float mass);
  void set_linear_damping(float damping);
  void set_angular_damping(float damping);
  void set_linear_velocity(const LVector3f &velocity);
  void set_angular_velocity(const LVector3f &velocity);
  void set_max_angular_velocity(float maximum);
  void set_sleep_linear_velocity(float velocity);
  void set_sleep_angular_velocity(float velocity);
  void set_solver_iteration_count(unsigned int count);
  void set_sleep_energy_threshold(float threshold);
  void set_sleep_damping(float damping);
  void set_mass_local_mat(const LMatrix4f &mat);
  void set_mass_space_inertia(const LVector3f &inertia);
  void set_flag(PhysxBodyFlag flag, bool value);
  void set_ccd_motion_threshold(float threshold);
  void set_wake_up_counter(float value);
  void set_contact_report_threshold(float threshold);

  float get_mass() const;
  float get_linear_damping() const;
  float get_angular_damping() const;
  LVector3f get_linear_velocity() const;
  LVector3f get_angular_velocity() const;
  float get_max_angular_velocity() const;
  float get_sleep_linear_velocity() const;
  float get_sleep_angular_velocity() const;
  unsigned int get_solver_iteration_count() const;
  float get_sleep_energy_threshold() const;
  float get_sleep_damping() const;
  LMatrix4f get_mass_local_mat() const;
  LVector3f get_mass_space_inertia() const;
  bool get_flag(PhysxBodyFlag flag) const;
  float get_ccd_motion_threshold() const;
  float get_wake_up_counter() const;
  float get_contact_report_threshold() const;

public:
  NxBodyDesc _desc;
};

#include "physxBodyDesc.I"

#endif // PHYSXBODYDESC_H
