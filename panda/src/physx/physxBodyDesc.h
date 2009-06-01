// Filename: physxBodyDesc.h
// Created by:  pratt (Apr 7, 2006)
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

#ifdef HAVE_PHYSX

#include "pandabase.h"

#include "physx_enumerations.h"
#include "physxManager.h"
#include "luse.h"

#include "NxPhysics.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxBodyDesc
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxBodyDesc {
PUBLISHED:
  PhysxBodyDesc();

  INLINE bool is_valid() const;
  INLINE void set_to_default();

  INLINE float get_angular_damping() const;
  LVecBase3f get_angular_velocity() const;
  INLINE float get_ccd_motion_threshold() const;
  INLINE unsigned int get_flags() const;
  INLINE float get_linear_damping() const;
  LVecBase3f get_linear_velocity() const;
  INLINE float get_mass() const;
  LMatrix4f get_mass_local_pose() const;
  LVecBase3f get_mass_space_inertia() const;
  INLINE float get_max_angular_velocity() const;
  INLINE float get_sleep_angular_velocity() const;
  INLINE float get_sleep_damping() const;
  INLINE float get_sleep_energy_threshold() const;
  INLINE float get_sleep_linear_velocity() const;
  INLINE unsigned int get_solver_iteration_count() const;
  INLINE float get_wake_up_counter() const;

  INLINE void set_angular_damping( float value );
  void set_angular_velocity( LVecBase3f value );
  INLINE void set_ccd_motion_threshold( float value );
  INLINE void set_flags( unsigned int value );
  INLINE void set_linear_damping( float value );
  void set_linear_velocity( LVecBase3f value );
  INLINE void set_mass( float value );
  void set_mass_local_pose( LMatrix4f value );
  void set_mass_space_inertia( LVecBase3f value );
  INLINE void set_max_angular_velocity( float value );
  INLINE void set_sleep_angular_velocity( float value );
  INLINE void set_sleep_damping(float value);
  INLINE void set_sleep_energy_threshold(float value);
  INLINE void set_sleep_linear_velocity( float value );
  INLINE void set_solver_iteration_count( unsigned int value );
  INLINE void set_wake_up_counter( float value );

public:
  NxBodyDesc nBodyDesc;
};

#include "physxBodyDesc.I"

#endif // HAVE_PHYSX

#endif // PHYSXBODYDESC_H
