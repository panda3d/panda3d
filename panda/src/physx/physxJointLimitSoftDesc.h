// Filename: physxJointLimitSoftDesc.h
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

#ifndef PHYSXJOINTLIMITSOFTDESC_H
#define PHYSXJOINTLIMITSOFTDESC_H

#ifdef HAVE_PHYSX

#include "pandabase.h"

#include "physx_enumerations.h"
#include "physxManager.h"
#include "luse.h"

#include "NxPhysics.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxJointLimitSoftDesc
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxJointLimitSoftDesc {
PUBLISHED:
  PhysxJointLimitSoftDesc();

  INLINE bool is_valid() const;
  INLINE void set_to_default();

  INLINE float get_damping() const;
  INLINE float get_restitution() const;
  INLINE float get_spring() const;
  INLINE float get_value() const;

  INLINE void set_damping( float value );
  INLINE void set_restitution( float value );
  INLINE void set_spring( float value );
  INLINE void set_value( float value );

public:
  NxJointLimitSoftDesc nJointLimitSoftDesc;
};

#include "physxJointLimitSoftDesc.I"

#endif // HAVE_PHYSX

#endif // PHYSXJOINTLIMITSOFTDESC_H
