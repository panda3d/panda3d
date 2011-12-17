// Filename: physxSceneDesc.h
// Created by:  enn0x (05Sep09)
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

#include "pandabase.h"
#include "luse.h"

#include "physxEnums.h"
#include "physx_includes.h"

class PhysxBounds3;

////////////////////////////////////////////////////////////////////
//       Class : PhysxSceneDesc
// Description : Descriptor for PhysxScene.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxSceneDesc : public PhysxEnums {

PUBLISHED:
  INLINE PhysxSceneDesc();
  INLINE ~PhysxSceneDesc();

  INLINE void set_to_default();
  INLINE bool is_valid() const;

  void set_flag(PhysxSceneFlag flag, bool value);
  void set_gravity(const LVector3f &gravity);
  void set_max_bounds(PhysxBounds3 &bounds);
  void set_static_structure(PhysxPruningStructure value);
  void set_dynamic_structure(PhysxPruningStructure value);
  void set_bp_type(PhysxBroadPhaseType value);
  void set_subdivision_level(unsigned int value);
  void set_num_grid_cells_x(unsigned int value);
  void set_num_grid_cells_y(unsigned int value);

  bool get_flag(PhysxSceneFlag flag) const;
  LVector3f get_gravity() const;
  PhysxBounds3 get_max_bounds() const;
  PhysxPruningStructure get_static_structure() const;
  PhysxPruningStructure get_dynamic_structure() const;
  PhysxBroadPhaseType get_bp_type() const;
  unsigned int get_subdivision_level() const;
  unsigned int get_num_grid_cells_x() const;
  unsigned int get_num_grid_cells_y() const;

public:
  NxSceneDesc _desc;
};

#include "physxSceneDesc.I"

#endif // PHYSXSCENEDESC_H
