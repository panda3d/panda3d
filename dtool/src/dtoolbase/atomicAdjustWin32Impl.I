// Filename: atomicAdjustWin32Impl.I
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


////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustWin32Impl::inc
//       Access: Public, Static
//  Description: Atomically increments the indicated variable.
////////////////////////////////////////////////////////////////////
INLINE void AtomicAdjustWin32Impl::
inc(PN_int32 &var) {
  InterlockedIncrement((LONG *)&var);
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustWin32Impl::dec
//       Access: Public, Static
//  Description: Atomically decrements the indicated variable and
//               returns true if the new value is nonzero, false if it
//               is zero.
////////////////////////////////////////////////////////////////////
INLINE bool AtomicAdjustWin32Impl::
dec(PN_int32 &var) {
  return (InterlockedDecrement((LONG *)&var) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustWin32Impl::set
//       Access: Public, Static
//  Description: Atomically changes the indicated variable and
//               returns the original value.
////////////////////////////////////////////////////////////////////
INLINE PN_int32 AtomicAdjustWin32Impl::
set(PN_int32 &var, PN_int32 new_value) {
  return InterlockedExchange((LONG *)&var, new_value);
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustWin32Impl::get
//       Access: Public, Static
//  Description: Atomically retrieves the snapshot value of the
//               indicated variable.  This is the only guaranteed safe
//               way to retrieve the value that other threads might be
//               asynchronously setting, incrementing, or decrementing
//               (via other AtomicAjust methods).
////////////////////////////////////////////////////////////////////
INLINE PN_int32 AtomicAdjustWin32Impl::
get(const PN_int32 &var) {
  return var;
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustWin32Impl::set_ptr
//       Access: Public, Static
//  Description: Atomically changes the indicated variable and
//               returns the original value.
////////////////////////////////////////////////////////////////////
INLINE void *AtomicAdjustWin32Impl::
set_ptr(void *&var, void *new_value) {
  void *orig_value = var;
  var = new_value;
  return orig_value;
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustWin32Impl::get_ptr
//       Access: Public, Static
//  Description: Atomically retrieves the snapshot value of the
//               indicated variable.  This is the only guaranteed safe
//               way to retrieve the value that other threads might be
//               asynchronously setting, incrementing, or decrementing
//               (via other AtomicAjust methods).
////////////////////////////////////////////////////////////////////
INLINE void *AtomicAdjustWin32Impl::
get_ptr(void * const &var) {
  return var;
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustWin32Impl::compare_and_exchange
//       Access: Public, Static
//  Description: Atomic compare and exchange.  
//
//               If mem is equal to old_value, store new_value in mem.
//               In either case, return the original value of mem.
//               The caller can test for success by comparing
//               return_value == old_value.
//
//               The atomic function expressed in pseudo-code:
//
//                 orig_value = mem;
//                 if (mem == old_value) {
//                   mem = new_value;
//                 }
//                 return orig_value;
//
////////////////////////////////////////////////////////////////////
INLINE PN_int32 AtomicAdjustWin32Impl::
compare_and_exchange(volatile PN_int32 &mem, PN_int32 old_value,
                     PN_int32 new_value) {
  // Note that the AtomicAdjust parameter order is different from
  // Windows convention!
  return InterlockedCompareExchange((volatile LONG *)&mem, new_value, old_value);
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustWin32Impl::compare_and_exchange_ptr
//       Access: Public, Static
//  Description: Atomic compare and exchange.  
//
//               As above, but works on pointers instead of integers.
////////////////////////////////////////////////////////////////////
INLINE void *AtomicAdjustWin32Impl::
compare_and_exchange_ptr(void * volatile &mem, void *old_value,
                         void *new_value) {
  // Note that the AtomicAdjust parameter order is different from
  // Windows convention!
  return InterlockedCompareExchangePointer(&mem, new_value, old_value);
}
