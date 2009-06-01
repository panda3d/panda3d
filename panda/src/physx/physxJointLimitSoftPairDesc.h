// Filename: physxJointLimitSoftPairDesc.h
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

#ifndef PHYSXJOINTLIMITSOFTPAIRDESC_H
#define PHYSXJOINTLIMITSOFTPAIRDESC_H

#ifdef HAVE_PHYSX

#include "pandabase.h"

#include "physx_enumerations.h"
#include "physxManager.h"
#include "luse.h"

class PhysxJointLimitSoftDesc;

#include "NxPhysics.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxJointLimitSoftPairDesc
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxJointLimitSoftPairDesc {
PUBLISHED:
  PhysxJointLimitSoftPairDesc();

  INLINE bool is_valid() const;
  INLINE void set_to_default();

  PhysxJointLimitSoftDesc & get_high() const;
  PhysxJointLimitSoftDesc & get_low() const;

  void set_high( PhysxJointLimitSoftDesc & value );
  void set_low( PhysxJointLimitSoftDesc & value );

public:
  NxJointLimitSoftPairDesc nJointLimitSoftPairDesc;
};

#include "physxJointLimitSoftPairDesc.I"

#endif // HAVE_PHYSX

#endif // PHYSXJOINTLIMITSOFTPAIRDESC_H
