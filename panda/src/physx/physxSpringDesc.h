// Filename: physxSpringDesc.h
// Created by:  enn0x (28Sep09)
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

#ifndef PHYSXSPRINGDESC_H
#define PHYSXSPRINGDESC_H

#include "pandabase.h"

#include "physx_includes.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxSpringDesc
// Description : Describes a joint spring. The spring is implicitly
//               integrated, so even high spring and damper
//               coefficients should be robust.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxSpringDesc {

PUBLISHED:
  INLINE PhysxSpringDesc();
  INLINE PhysxSpringDesc(float spring, float damper=0, float targetValue=0);
  INLINE ~PhysxSpringDesc();

  void set_spring(float spring);
  void set_damper(float damper);
  void set_target_value(float target);

  float get_spring() const;
  float get_damper() const;
  float get_target_value() const;

public:
  NxSpringDesc _desc;
};

#include "physxSpringDesc.I"

#endif // PHYSXSPRINGDESC_H
