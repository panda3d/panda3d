// Filename: atomicAdjustWin32Impl.I
// Created by:  drose (07Feb06)
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


////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustWin32Impl::inc
//       Access: Public, Static
//  Description: Atomically increments the indicated variable.
////////////////////////////////////////////////////////////////////
INLINE void AtomicAdjustWin32Impl::
inc(TVOLATILE AtomicAdjustWin32Impl::Integer &var) {
  InterlockedIncrement(&var);
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustWin32Impl::dec
//       Access: Public, Static
//  Description: Atomically decrements the indicated variable and
//               returns true if the new value is nonzero, false if it
//               is zero.
////////////////////////////////////////////////////////////////////
INLINE bool AtomicAdjustWin32Impl::
dec(TVOLATILE AtomicAdjustWin32Impl::Integer &var) {
  return (InterlockedDecrement(&var) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustWin32Impl::add
//       Access: Public, Static
//  Description: Atomically computes var += delta.  It is legal for
//               delta to be negative.
////////////////////////////////////////////////////////////////////
INLINE void AtomicAdjustWin32Impl::
add(TVOLATILE AtomicAdjustWin32Impl::Integer &var, AtomicAdjustWin32Impl::Integer delta) {
  AtomicAdjustWin32Impl::Integer orig_value = var;
  while (compare_and_exchange(var, orig_value, orig_value + delta) != orig_value) {
    orig_value = var;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustWin32Impl::set
//       Access: Public, Static
//  Description: Atomically changes the indicated variable and
//               returns the original value.
////////////////////////////////////////////////////////////////////
INLINE AtomicAdjustWin32Impl::Integer AtomicAdjustWin32Impl::
set(TVOLATILE AtomicAdjustWin32Impl::Integer &var, AtomicAdjustWin32Impl::Integer new_value) {
  return InterlockedExchange(&var, new_value);
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
INLINE AtomicAdjustWin32Impl::Integer AtomicAdjustWin32Impl::
get(const TVOLATILE AtomicAdjustWin32Impl::Integer &var) {
  return var;
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustWin32Impl::set_ptr
//       Access: Public, Static
//  Description: Atomically changes the indicated variable and
//               returns the original value.
////////////////////////////////////////////////////////////////////
INLINE void *AtomicAdjustWin32Impl::
set_ptr(void * TVOLATILE &var, void *new_value) {
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
get_ptr(void * const TVOLATILE &var) {
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
INLINE AtomicAdjustWin32Impl::Integer AtomicAdjustWin32Impl::
compare_and_exchange(TVOLATILE AtomicAdjustWin32Impl::Integer &mem, AtomicAdjustWin32Impl::Integer old_value,
                     AtomicAdjustWin32Impl::Integer new_value) {
  // Note that the AtomicAdjust parameter order is different from
  // Windows convention!
  return InterlockedCompareExchange((TVOLATILE LONG *)&mem, new_value, old_value);
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustWin32Impl::compare_and_exchange_ptr
//       Access: Public, Static
//  Description: Atomic compare and exchange.  
//
//               As above, but works on pointers instead of integers.
////////////////////////////////////////////////////////////////////
INLINE void *AtomicAdjustWin32Impl::
compare_and_exchange_ptr(void * TVOLATILE &mem, void *old_value,
                         void *new_value) {
  // Note that the AtomicAdjust parameter order is different from
  // Windows convention!
  return InterlockedCompareExchangePointer(&mem, new_value, old_value);
}
