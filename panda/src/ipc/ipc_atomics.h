// Filename: ipc_atomics.h
// Created by:  frang (10Feb00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef __IPC_ATOMICS_H__
#define __IPC_ATOMICS_H__

#include <pandabase.h>

#include "ipc_traits.h"

INLINE int set_atomic(int& var, int val) {
  return ipc_traits::set_atomic(var, val);
}

INLINE int inc_atomic(int& var) {
  return ipc_traits::inc_atomic(var);
}

INLINE int dec_atomic(int& var) {
  return ipc_traits::dec_atomic(var);
}

#endif /* __IPC_ATOMICS_H__ */
