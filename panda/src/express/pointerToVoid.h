// Filename: pointerToVoid.h
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

#ifndef POINTERTOVOID_H
#define POINTERTOVOID_H

#include "pandabase.h"
#include "notify.h"

////////////////////////////////////////////////////////////////////
//       Class : PointerToVoid
// Description : This is the non-template part of the base class for
//               PointerTo and ConstPointerTo.  It is necessary so we
//               can keep a pointer to a non-template class within the
//               ReferenceCount object, to implement weak reference
//               pointers--we need to have something to clean up when
//               the ReferenceCount object destructs.
//
//               This is the base class for PointerToBase<T>.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS PointerToVoid {
protected:
  INLINE PointerToVoid();
  INLINE ~PointerToVoid();

PUBLISHED:
  INLINE bool is_null() const;

protected:
  // Within the PointerToVoid class, we only store a void pointer.
  // This is actually the (To *) pointer that is typecast to (void *)
  // from the derived template classes.

  // It is tempting to try to store a (ReferenceCount *) pointer here,
  // but this is not useful because it prohibits defining, say,
  // PT(PandaNode), or a PointerTo any class that inherits virtually
  // from ReferenceCount.  (You can't downcast past a virtual
  // inheritance level, but you can always cross-cast from a void
  // pointer.)
  void *_void_ptr;
};

#include "pointerToVoid.I"

#endif
