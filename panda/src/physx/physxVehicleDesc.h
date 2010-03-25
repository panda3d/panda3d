// Filename: physxVehicleDesc.h
// Created by:  enn0x (23Mar10)
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

#ifndef PHYSXVEHICLEDESC_H
#define PHYSXVEHICLEDESC_H

#include "pandabase.h"

#include "physx_includes.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxVehicleDesc
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxVehicleDesc : public ReferenceCount {

PUBLISHED:
  INLINE PhysxVehicleDesc();
  INLINE ~PhysxVehicleDesc();

  INLINE void set_to_default();
  INLINE bool is_valid() const;

public:
};

#include "physxVehicleDesc.I"

#endif // PHYSXVEHICLEDESC_H
