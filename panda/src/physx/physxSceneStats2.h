// Filename: physxSceneStats2.h
// Created by:  enn0x (20Oct09)
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

#ifndef PHYSXSCENESTATS2_H
#define PHYSXSCENESTATS2_H

#include "pandabase.h"

#include "physx_includes.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxSceneStats2
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxSceneStats2 {

PUBLISHED:
  PhysxSceneStats2(const NxSceneStats2 *ptr);
  ~PhysxSceneStats2();

  INLINE unsigned int get_num_stats() const;
  INLINE int get_cur_value(unsigned int index) const;
  INLINE int get_max_value(unsigned int index) const;
  INLINE const char *get_name(unsigned int index) const;
  INLINE unsigned int get_parent(unsigned int index) const;

private:
  const NxSceneStats2 *_ptr;
};

#include "physxSceneStats2.I"

#endif // PHYSXSCENESTATS2_H
