// Filename: physxConstraintDominance.h
// Created by:  enn0x (22Dec09)
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

#ifndef PHYSXCONSTRAINTDOMINANCE_H
#define PHYSXCONSTRAINTDOMINANCE_H

#include "pandabase.h"
#include "luse.h"

#include "config_physx.h"

class PhysxShape;

////////////////////////////////////////////////////////////////////
//       Class : PhysxConstraintDominance
// Description : Expresses the dominance relationship of a
//               constraint. For the time being only three settings
//               are permitted:. (1.0f, 1.0f), (0.0f, 1.0f), and
//               (1.0f, 0.0f). 
//
//               See PhysxScene::set_dominance_group_pair for a
//               detailed explanation of dominance behaviour.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxConstraintDominance {

PUBLISHED:
  INLINE PhysxConstraintDominance(float d0, float d1);
  INLINE ~PhysxConstraintDominance();

  float get_0() const;
  float get_1() const;
  void set_0(float d0);
  void set_1(float d1);

public:
  INLINE void set_dominance(NxConstraintDominance value);
  INLINE NxConstraintDominance get_dominance() const;

private:
  NxConstraintDominance _dominance;
};

#include "physxConstraintDominance.I"

#endif // PHYSXCONSTRAINTDOMINANCE_H
