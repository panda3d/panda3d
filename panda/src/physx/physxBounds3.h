// Filename: physxBounds3.h
// Created by:  pratt (Dec 12, 2007)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2006, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef PHYSXBOUNDS3_H
#define PHYSXBOUNDS3_H

#ifdef HAVE_PHYSX

#include "pandabase.h"

#include "physx_enumerations.h"
#include "physxManager.h"
#include "luse.h"

#include "NxPhysics.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxBounds3
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxBounds3 {
PUBLISHED:
  PhysxBounds3();
  ~PhysxBounds3();

  INLINE void bounds_of_obb(const LMatrix3f & orientation, const LVecBase3f & translation, const LVecBase3f & half_dims);
  INLINE void combine(const PhysxBounds3 & b2);
  INLINE bool contain(const LVecBase3f & v) const;
  INLINE void fatten(float distance);
  INLINE void get_dimensions(LVecBase3f & dims) const;
  INLINE void include(const LVecBase3f & v);
  INLINE bool intersects(const PhysxBounds3 & b) const;
  INLINE bool intersects2d(const PhysxBounds3 & b, unsigned axis_to_ignore) const;
  INLINE bool is_empty() const;
  INLINE void scale(float scale);
  INLINE void set(const LVecBase3f & min, const LVecBase3f & max);
  INLINE void set_center_extents(const LVecBase3f & c, const LVecBase3f & e);
  INLINE void set_empty();
  INLINE void set_infinite();
  INLINE void transform(const LMatrix3f & orientation, const LVecBase3f & translation);

  LVecBase3f get_max() const;
  LVecBase3f get_min() const;

  void set_max(LVecBase3f value);
  void set_min(LVecBase3f value);

public:
  NxBounds3 *nBounds3;
};

#include "physxBounds3.I"

#endif // HAVE_PHYSX

#endif // PHYSXBOUNDS3_H
