// Filename: atomicAdjustWin32Impl.h
// Created by:  drose (07Feb06)
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

#ifndef ATOMICADJUSTWIN32IMPL_H
#define ATOMICADJUSTWIN32IMPL_H

#include "pandabase.h"
#include "selectThreadImpl.h"

#ifdef THREAD_WIN32_IMPL

#include "notify.h"
#include "numeric_types.h"

#include <windows.h>

////////////////////////////////////////////////////////////////////
//       Class : AtomicAdjustWin32Impl
// Description : Uses Windows native calls to implement atomic
//               adjustments.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS AtomicAdjustWin32Impl {
public:
  INLINE static PN_int32 inc(PN_int32 &var);
  INLINE static PN_int32 dec(PN_int32 &var);
  INLINE static PN_int32 set(PN_int32 &var, PN_int32 new_value);
  INLINE static PN_int32 get(const PN_int32 &var);
};

#include "atomicAdjustWin32Impl.I"

#endif  // THREAD_WIN32_IMPL

#endif
