// Filename: weakPointerToVoid.h
// Created by:  drose (27Sep04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
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

#ifndef WEAKPOINTERTOVOID_H
#define WEAKPOINTERTOVOID_H

#include "pandabase.h"
#include "pointerToVoid.h"

////////////////////////////////////////////////////////////////////
//       Class : WeakPointerToVoid
// Description : This is the specialization of PointerToVoid for weak
//               pointers.  It needs an additional flag to indicate
//               that the pointer has been deleted.
////////////////////////////////////////////////////////////////////
class WeakPointerToVoid : public PointerToVoid {
protected:
  INLINE WeakPointerToVoid();

public:
  INLINE void mark_deleted();

PUBLISHED:
  INLINE bool was_deleted() const;

protected:
  bool _ptr_was_deleted;
};

#include "weakPointerToVoid.I"

#endif
