/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file atomicAdjustDummyImpl.I
 * @author drose
 * @date 2002-08-09
 */

/**
 * Atomically increments the indicated variable.
 */
ALWAYS_INLINE void AtomicAdjustDummyImpl::
inc(TVOLATILE AtomicAdjustDummyImpl::Integer &var) {
  ++var;
}

/**
 * Atomically decrements the indicated variable and returns true if the new
 * value is nonzero, false if it is zero.
 */
ALWAYS_INLINE bool AtomicAdjustDummyImpl::
dec(TVOLATILE AtomicAdjustDummyImpl::Integer &var) {
  return (--var) != 0;
}

/**
 * Atomically computes var += delta.  It is legal for delta to be negative.
 * Returns the result of the addition.
 */
ALWAYS_INLINE AtomicAdjustDummyImpl::Integer AtomicAdjustDummyImpl::
add(TVOLATILE AtomicAdjustDummyImpl::Integer &var, AtomicAdjustDummyImpl::Integer delta) {
  Integer new_value = var + delta;
  var = new_value;
  return new_value;
}

/**
 * Atomically changes the indicated variable and returns the original value.
 */
ALWAYS_INLINE AtomicAdjustDummyImpl::Integer AtomicAdjustDummyImpl::
set(TVOLATILE AtomicAdjustDummyImpl::Integer &var, AtomicAdjustDummyImpl::Integer new_value) {
  Integer orig_value = var;
  var = new_value;
  return orig_value;
}

/**
 * Atomically retrieves the snapshot value of the indicated variable.  This is
 * the only guaranteed safe way to retrieve the value that other threads might
 * be asynchronously setting, incrementing, or decrementing (via other
 * AtomicAjust methods).
 */
ALWAYS_INLINE AtomicAdjustDummyImpl::Integer AtomicAdjustDummyImpl::
get(const TVOLATILE AtomicAdjustDummyImpl::Integer &var) {
  return var;
}

/**
 * Atomically changes the indicated variable and returns the original value.
 */
ALWAYS_INLINE AtomicAdjustDummyImpl::Pointer AtomicAdjustDummyImpl::
set_ptr(TVOLATILE AtomicAdjustDummyImpl::Pointer &var,
        AtomicAdjustDummyImpl::Pointer new_value) {
  Pointer orig_value = var;
  var = new_value;
  return orig_value;
}

/**
 * Atomically retrieves the snapshot value of the indicated variable.  This is
 * the only guaranteed safe way to retrieve the value that other threads might
 * be asynchronously setting, incrementing, or decrementing (via other
 * AtomicAjust methods).
 */
ALWAYS_INLINE AtomicAdjustDummyImpl::Pointer AtomicAdjustDummyImpl::
get_ptr(const TVOLATILE AtomicAdjustDummyImpl::Pointer &var) {
  return var;
}

/**
 * Atomic compare and exchange.
 *
 * If mem is equal to old_value, store new_value in mem.  In either case,
 * return the original value of mem.  The caller can test for success by
 * comparing return_value == old_value.
 */
ALWAYS_INLINE AtomicAdjustDummyImpl::Integer AtomicAdjustDummyImpl::
compare_and_exchange(TVOLATILE AtomicAdjustDummyImpl::Integer &mem,
                     AtomicAdjustDummyImpl::Integer old_value,
                     AtomicAdjustDummyImpl::Integer new_value) {
  Integer orig_value = mem;
  if (mem == old_value) {
    mem = new_value;
  }
  return orig_value;
}

/**
 * Atomic compare and exchange.
 *
 * As above, but works on pointers instead of integers.
 */
ALWAYS_INLINE AtomicAdjustDummyImpl::Pointer AtomicAdjustDummyImpl::
compare_and_exchange_ptr(TVOLATILE AtomicAdjustDummyImpl::Pointer &mem,
                         AtomicAdjustDummyImpl::Pointer old_value,
                         AtomicAdjustDummyImpl::Pointer new_value) {
  Pointer orig_value = mem;
  if (mem == old_value) {
    mem = new_value;
  }
  return orig_value;
}
