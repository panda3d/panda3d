/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file weakPointerToVoid.h
 * @author drose
 * @date 2004-09-27
 */

#ifndef WEAKPOINTERTOVOID_H
#define WEAKPOINTERTOVOID_H

#include "pandabase.h"
#include "pointerToVoid.h"
#include "weakPointerCallback.h"
#include "weakReferenceList.h"

/**
 * This is the specialization of PointerToVoid for weak pointers.  It needs an
 * additional flag to indicate that the pointer has been deleted.
 */
class EXPCL_PANDA_EXPRESS WeakPointerToVoid : public PointerToVoid {
protected:
  constexpr WeakPointerToVoid() noexcept = default;

public:
  INLINE void add_callback(WeakPointerCallback *callback) const;
  INLINE void remove_callback(WeakPointerCallback *callback) const;

PUBLISHED:
  INLINE bool was_deleted() const;
  INLINE bool is_valid_pointer() const;

protected:
  mutable WeakReferenceList *_weak_ref = nullptr;
};

#include "weakPointerToVoid.I"

#endif
