/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcast.h
 * @author drose
 * @date 2001-08-06
 */

#ifndef DCAST_H
#define DCAST_H

#include "pandabase.h"

#include "typeHandle.h"
#include "typedObject.h"
#include "config_express.h"

// The DCAST (downcast) macro is defined as a convenience for downcasting from
// some TypedObject pointer (or a PointerTo).  It's just a normal C++-style
// downcast, except it first checks get_type() to make sure the downcasting is
// safe.  If you compile with NDEBUG, or set verify-dcast to #f, this check is
// removed.

// DCAST will return NULL if the downcasting is unsafe.  If you'd rather it
// abort out of the function (a la nassertvnassertr), then see DCAST_INTO_V
// and DCAST_INTO_R, below.

template<class WantType>
INLINE WantType *_dcast(WantType *, TypedObject *ptr);
template<class WantType>
INLINE const WantType *_dcast(WantType *, const TypedObject *ptr);

// Note: it is important that DCAST not repeat the pointer parameter, since
// many users of DCAST may want to use the result of a function as the pointer
// parameter, and it could be terribly confusing and difficult to trace if the
// function were inadvertently executed twice.  This happened!
#define DCAST(want_type, pointer) _dcast((want_type*)0, pointer)

// DCAST_INTO_V and DCAST_INTO_R are similar in purpose to DCAST, except they:
// (a) automatically assign a variable instead of returning the downcasted
// pointer, and (b) they immediately return out of the function if the
// downcasting fails.  DCAST_INTO_V is for use in a void function and returns
// nothing; DCAST_INTO_R is for use in a non-void function and returns the
// indicated value.

// Both DCAST_INTO_V and DCAST_INTO_R accept as the first parameter a variable
// of type (want_type *) or (const want_type *), instead of the name of the
// type.  This variable will be filled with the new pointer.


// _dcast_ref is used to implement DCAST_INTO_V and DCAST_INTO_R.  Its
// difference from _dcast is that it takes a reference to a pointer as a first
// parameter.  The main point of this is to shut up the compiler about
// pointers used before their value is assigned.
template<class WantType>
INLINE WantType *_dcast_ref(WantType *&, TypedObject *ptr);
template<class WantType>
INLINE const WantType *_dcast_ref(WantType *&, const TypedObject *ptr);

// _dcast_verify performs the actual verification.  This is an empty function
// when DO_DCAST is not set, but we still need to define it for ABI
// compatibility reasons.
EXPCL_PANDA_EXPRESS bool
_dcast_verify(TypeHandle want_handle, size_t want_size,
              const TypedObject *ptr);

#define DCAST_INTO_V(to_pointer, from_pointer) \
  { \
    (to_pointer) = _dcast_ref(to_pointer, from_pointer); \
    nassertv((void *)(to_pointer) != nullptr); \
  }

#define DCAST_INTO_R(to_pointer, from_pointer, return_value) \
  { \
    (to_pointer) = _dcast_ref(to_pointer, from_pointer); \
    nassertr((void *)(to_pointer) != nullptr, return_value); \
  }

#include "dcast.T"

#endif
