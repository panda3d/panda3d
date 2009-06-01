// Filename: physxBox.h
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

#ifndef PHYSXBOX_H
#define PHYSXBOX_H

#ifdef HAVE_PHYSX

#include "pandabase.h"

#include "physx_enumerations.h"
#include "physxManager.h"
#include "luse.h"

#include "NxPhysics.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxBox
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxBox {
PUBLISHED:
  PhysxBox();
  PhysxBox(const LVecBase3f & _center, const LVecBase3f & _extents, const LMatrix3f & _rot);
  ~PhysxBox();

  INLINE bool is_valid() const;
  INLINE void rotate(const LMatrix4f & mtx, PhysxBox & obb) const;
  INLINE void set_empty();

  LVecBase3f get_center() const;
  LVecBase3f get_extents() const;
  LMatrix3f get_rot() const;

  void set_center( LVecBase3f value );
  void set_extents( LVecBase3f value );
  void set_rot( LMatrix3f value );

public:
  NxBox *nBox;
};

#include "physxBox.I"

#endif // HAVE_PHYSX

#endif // PHYSXBOX_H
