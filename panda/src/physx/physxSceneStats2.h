// Filename: physxSceneStats2.h
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

#ifndef PHYSXSCENESTATS2_H
#define PHYSXSCENESTATS2_H

#ifdef HAVE_PHYSX

#include "pandabase.h"

#include "physx_enumerations.h"
#include "physxManager.h"
#include "luse.h"

#include "NxPhysics.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxSceneStats2
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxSceneStats2 {
PUBLISHED:
  PhysxSceneStats2(const NxSceneStats2 *stats2);
  ~PhysxSceneStats2();

  INLINE unsigned int get_num_stats() const;
  INLINE int get_cur_value(unsigned int index) const;
  INLINE int get_max_value(unsigned int index) const;
  INLINE const char *get_name(unsigned int index) const;
  INLINE unsigned int get_parent(unsigned int index) const;

private:
  const NxSceneStats2 *nSceneStats2;
};

#include "physxSceneStats2.I"

#endif // HAVE_PHYSX

#endif // PHYSXSCENESTATS2_H
