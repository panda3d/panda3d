// Filename: atomicAdjustDummyImpl.I
// Created by:  drose (09Aug02)
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
//     Function: AtomicAdjustDummyImpl::inc
//       Access: Public, Static
//  Description: Atomically increments the indicated variable.
////////////////////////////////////////////////////////////////////
INLINE void AtomicAdjustDummyImpl::
inc(TVOLATILE AtomicAdjustDummyImpl::Integer &var) {
  ++var;
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustDummyImpl::dec
//       Access: Public, Static
//  Description: Atomically decrements the indicated variable and
//               returns true if the new value is nonzero, false if it
//               is zero.
////////////////////////////////////////////////////////////////////
INLINE bool AtomicAdjustDummyImpl::
dec(TVOLATILE AtomicAdjustDummyImpl::Integer &var) {
  return (--var) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustDummyImpl::add
//       Access: Public, Static
//  Description: Atomically computes var += delta.  It is legal for
//               delta to be negative.
////////////////////////////////////////////////////////////////////
INLINE void AtomicAdjustDummyImpl::
add(TVOLATILE AtomicAdjustDummyImpl::Integer &var, AtomicAdjustDummyImpl::Integer delta) {
  var += delta;
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustDummyImpl::set
//       Access: Public, Static
//  Description: Atomically changes the indicated variable and
//               returns the original value.
////////////////////////////////////////////////////////////////////
INLINE AtomicAdjustDummyImpl::Integer AtomicAdjustDummyImpl::
set(TVOLATILE AtomicAdjustDummyImpl::Integer &var, AtomicAdjustDummyImpl::Integer new_value) {
  Integer orig_value = var;
  var = new_value;
  return orig_value;
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustDummyImpl::get
//       Access: Public, Static
//  Description: Atomically retrieves the snapshot value of the
//               indicated variable.  This is the only guaranteed safe
//               way to retrieve the value that other threads might be
//               asynchronously setting, incrementing, or decrementing
//               (via other AtomicAjust methods).
////////////////////////////////////////////////////////////////////
INLINE AtomicAdjustDummyImpl::Integer AtomicAdjustDummyImpl::
get(const TVOLATILE AtomicAdjustDummyImpl::Integer &var) {
  return var;
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustDummyImpl::set_ptr
//       Access: Public, Static
//  Description: Atomically changes the indicated variable and
//               returns the original value.
////////////////////////////////////////////////////////////////////
INLINE AtomicAdjustDummyImpl::Pointer AtomicAdjustDummyImpl::
set_ptr(TVOLATILE AtomicAdjustDummyImpl::Pointer &var, 
        AtomicAdjustDummyImpl::Pointer new_value) {
  Pointer orig_value = var;
  var = new_value;
  return orig_value;
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustDummyImpl::get_ptr
//       Access: Public, Static
//  Description: Atomically retrieves the snapshot value of the
//               indicated variable.  This is the only guaranteed safe
//               way to retrieve the value that other threads might be
//               asynchronously setting, incrementing, or decrementing
//               (via other AtomicAjust methods).
////////////////////////////////////////////////////////////////////
INLINE AtomicAdjustDummyImpl::Pointer AtomicAdjustDummyImpl::
get_ptr(const TVOLATILE AtomicAdjustDummyImpl::Pointer &var) {
  return var;
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustDummyImpl::compare_and_exchange
//       Access: Public, Static
//  Description: Atomic compare and exchange.  
//
//               If mem is equal to old_value, store new_value in mem.
//               In either case, return the original value of mem.
//               The caller can test for success by comparing
//               return_value == old_value.
////////////////////////////////////////////////////////////////////
INLINE AtomicAdjustDummyImpl::Integer AtomicAdjustDummyImpl::
compare_and_exchange(TVOLATILE AtomicAdjustDummyImpl::Integer &mem, 
                     AtomicAdjustDummyImpl::Integer old_value,
                     AtomicAdjustDummyImpl::Integer new_value) {
  Integer orig_value = mem;
  if (mem == old_value) {
    mem = new_value;
  }
  return orig_value;
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustDummyImpl::compare_and_exchange_ptr
//       Access: Public, Static
//  Description: Atomic compare and exchange.  
//
//               As above, but works on pointers instead of integers.
////////////////////////////////////////////////////////////////////
INLINE AtomicAdjustDummyImpl::Pointer AtomicAdjustDummyImpl::
compare_and_exchange_ptr(TVOLATILE AtomicAdjustDummyImpl::Pointer &mem, 
                         AtomicAdjustDummyImpl::Pointer old_value,
                         AtomicAdjustDummyImpl::Pointer new_value) {
  Pointer orig_value = mem;
  if (mem == old_value) {
    mem = new_value;
  }
  return orig_value;
}
