// Filename: physxActorDesc.h
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

#ifndef PHYSXACTORDESC_H
#define PHYSXACTORDESC_H

#ifdef HAVE_PHYSX

#include "pandabase.h"

#include "physx_enumerations.h"
#include "physxManager.h"
#include "luse.h"

class PhysxShapeDesc;
class PhysxBodyDesc;

#include "NxPhysics.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxActorDesc
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxActorDesc {
PUBLISHED:
  PhysxActorDesc();

  INLINE bool is_valid() const;
  INLINE void set_to_default();

  void add_shape(PhysxShapeDesc pShapeDesc);

  const PhysxBodyDesc * get_body() const;
  INLINE float get_density() const;
  INLINE unsigned int get_flags() const;
  LMatrix4f get_global_pose() const;

  void set_body( const PhysxBodyDesc * value );
  INLINE void set_density( float value );
  INLINE void set_flags( unsigned int value );
  void set_global_pose( LMatrix4f value );

public:
  NxActorDesc nActorDesc;
};

#include "physxActorDesc.I"

#endif // HAVE_PHYSX

#endif // PHYSXACTORDESC_H
