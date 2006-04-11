// Filename: atomicAdjustPosixImpl.h
// Created by:  drose (10Feb06)
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

#ifndef ATOMICADJUSTPOSIXIMPL_H
#define ATOMICADJUSTPOSIXIMPL_H

#include "dtoolbase.h"
#include "selectThreadImpl.h"

#ifdef THREAD_POSIX_IMPL

#include "numeric_types.h"

#include <pthread.h>

////////////////////////////////////////////////////////////////////
//       Class : AtomicAdjustPosixImpl
// Description : Uses POSIX to implement atomic adjustments.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOL AtomicAdjustPosixImpl {
public:
  INLINE static void inc(PN_int32 &var);
  INLINE static bool dec(PN_int32 &var);
  INLINE static PN_int32 set(PN_int32 &var, PN_int32 new_value);
  INLINE static PN_int32 get(const PN_int32 &var);

  INLINE static void *set_ptr(void *&var, void *new_value);
  INLINE static void *get_ptr(void * const &var);

private:
  static pthread_mutex_t _mutex;
};

#include "atomicAdjustPosixImpl.I"

#endif  // THREAD_POSIX_IMPL

#endif
