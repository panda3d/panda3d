// Filename: ipc_atomics.h
// Created by:  frang (10Feb00)
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
