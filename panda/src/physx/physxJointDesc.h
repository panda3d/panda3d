// Filename: physxJointDesc.h
// Created by:  pratt (Jun 20, 2006)
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

#ifndef PHYSXJOINTDESC_H
#define PHYSXJOINTDESC_H

#ifdef HAVE_PHYSX

#include "pandabase.h"

#include "physx_enumerations.h"
#include "physxManager.h"
#include "luse.h"

class PhysxActorNode;

#include "NxPhysics.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxJointDesc
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxJointDesc {
PUBLISHED:
  ~PhysxJointDesc();

  INLINE bool is_valid() const;
  void set_global_anchor(const LVecBase3f & ws_anchor);
  void set_global_axis(const LVecBase3f & ws_axis);
  INLINE void set_to_default();

  INLINE unsigned int get_joint_flags() const;
  LVecBase3f get_local_anchor(int index) const;
  LVecBase3f get_local_axis(int index) const;
  LVecBase3f get_local_normal(int index) const;
  INLINE float get_max_force() const;
  INLINE float get_max_torque() const;
  INLINE const char * get_name() const;

  void set_actor(int index, PhysxActorNode * value);
  INLINE void set_joint_flags(unsigned int value);
  void set_local_anchor(int index, LVecBase3f value);
  void set_local_axis(int index, LVecBase3f value);
  void set_local_normal(int index, LVecBase3f value);
  INLINE void set_max_force(float value);
  INLINE void set_max_torque(float value);
  INLINE void set_name(const char * value);

public:
  NxJointDesc *nJointDesc;

protected:
  PhysxJointDesc(NxJointDesc *subJointDesc);

private:
  string _name_store;
};

#include "physxJointDesc.I"

#endif // HAVE_PHYSX

#endif // PHYSXJOINTDESC_H
