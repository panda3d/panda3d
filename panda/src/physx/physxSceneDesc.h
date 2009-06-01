// Filename: physxSceneDesc.h
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

#ifndef PHYSXSCENEDESC_H
#define PHYSXSCENEDESC_H

#ifdef HAVE_PHYSX

#include "pandabase.h"

#include "physx_enumerations.h"
#include "physxManager.h"
#include "luse.h"

class PhysxBounds3;

#include "NxPhysics.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxSceneDesc
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxSceneDesc {
PUBLISHED:
  PhysxSceneDesc();

  INLINE bool is_valid() const;
  INLINE void set_to_default();

  INLINE unsigned int get_background_thread_count() const;
  INLINE unsigned int get_background_thread_mask() const;
  INLINE bool get_bounds_planes() const;
  PhysxPruningStructure get_dynamic_structure() const;
  INLINE unsigned int get_flags() const;
  LVecBase3f get_gravity() const;
  INLINE bool get_ground_plane() const;
  INLINE unsigned int get_internal_thread_count() const;
  PhysxBounds3 * get_max_bounds() const;
  INLINE unsigned int get_max_iter() const;
  INLINE float get_max_timestep() const;
  INLINE unsigned int get_sim_thread_mask() const;
  PhysxThreadPriority get_sim_thread_priority() const;
  INLINE unsigned int get_sim_thread_stack_size() const;
  PhysxSimulationType get_sim_type() const;
  PhysxPruningStructure get_static_structure() const;
  INLINE unsigned int get_subdivision_level() const;
  INLINE unsigned int get_thread_mask() const;
  PhysxTimeStepMethod get_time_step_method() const;
  INLINE unsigned int get_up_axis() const;
  PhysxThreadPriority get_worker_thread_priority() const;
  INLINE unsigned int get_worker_thread_stack_size() const;

  INLINE void set_background_thread_count( unsigned int value );
  INLINE void set_background_thread_mask( unsigned int value );
  INLINE void set_bounds_planes( bool value );
  void set_dynamic_structure(PhysxPruningStructure value);
  INLINE void set_flags( unsigned int value );
  void set_gravity( LVecBase3f value );
  INLINE void set_ground_plane( bool value );
  INLINE void set_internal_thread_count( unsigned int value );
  void set_max_bounds(PhysxBounds3 * value);
  INLINE void set_max_iter( unsigned int value );
  INLINE void set_max_timestep( float value );
  INLINE void set_sim_thread_mask(unsigned int value);
  void set_sim_thread_priority(PhysxThreadPriority value);
  INLINE void set_sim_thread_stack_size(unsigned int value);
  void set_sim_type( PhysxSimulationType value );
  void set_static_structure(PhysxPruningStructure value);
  INLINE void set_subdivision_level(unsigned int value);
  INLINE void set_thread_mask( unsigned int value );
  void set_time_step_method( PhysxTimeStepMethod value );
  INLINE void set_up_axis(unsigned int value);
  void set_worker_thread_priority(PhysxThreadPriority value);
  INLINE void set_worker_thread_stack_size(unsigned int value);

public:
  NxSceneDesc nSceneDesc;
};

#include "physxSceneDesc.I"

#endif // HAVE_PHYSX

#endif // PHYSXSCENEDESC_H
