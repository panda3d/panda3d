// Filename: physxJointDriveDesc.h
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

#ifndef PHYSXJOINTDRIVEDESC_H
#define PHYSXJOINTDRIVEDESC_H

#ifdef HAVE_PHYSX

#include "pandabase.h"

#include "physx_enumerations.h"
#include "physxManager.h"
#include "luse.h"

#include "NxPhysics.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxJointDriveDesc
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxJointDriveDesc {
PUBLISHED:
  PhysxJointDriveDesc();


  INLINE float get_damping() const;
  INLINE float get_force_limit() const;
  INLINE float get_spring() const;

  INLINE void set_damping( float value );
  INLINE void set_force_limit( float value );
  INLINE void set_spring( float value );

public:
  NxJointDriveDesc nJointDriveDesc;
};

#include "physxJointDriveDesc.I"

#endif // HAVE_PHYSX

#endif // PHYSXJOINTDRIVEDESC_H
