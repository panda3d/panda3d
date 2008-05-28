// Filename: weakPointerToVoid.h
// Created by:  drose (27Sep04)
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

#ifndef WEAKPOINTERTOVOID_H
#define WEAKPOINTERTOVOID_H

#include "pandabase.h"
#include "pointerToVoid.h"
#include "weakPointerCallback.h"

////////////////////////////////////////////////////////////////////
//       Class : WeakPointerToVoid
// Description : This is the specialization of PointerToVoid for weak
//               pointers.  It needs an additional flag to indicate
//               that the pointer has been deleted.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS WeakPointerToVoid : public PointerToVoid {
protected:
  INLINE WeakPointerToVoid();
  INLINE ~WeakPointerToVoid();

public:
  INLINE void mark_deleted();

  INLINE void set_callback(WeakPointerCallback *callback);
  INLINE WeakPointerCallback *get_callback() const;

PUBLISHED:
  INLINE bool was_deleted() const;
  INLINE bool is_valid_pointer() const;

protected:
  bool _ptr_was_deleted;
  WeakPointerCallback *_callback;
};

#include "weakPointerToVoid.I"

#endif
