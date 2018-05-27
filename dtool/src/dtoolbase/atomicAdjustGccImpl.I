/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file atomicAdjustGccImpl.I
 * @author rdb
 * @date 2014-07-04
 */

/**
 * Atomically increments the indicated variable.
 */
INLINE void AtomicAdjustGccImpl::
inc(TVOLATILE AtomicAdjustGccImpl::Integer &var) {
  __atomic_fetch_add(&var, 1, __ATOMIC_SEQ_CST);
}

/**
 * Atomically decrements the indicated variable and returns true if the new
 * value is nonzero, false if it is zero.
 */
INLINE bool AtomicAdjustGccImpl::
dec(TVOLATILE AtomicAdjustGccImpl::Integer &var) {
  return (__atomic_sub_fetch(&var, 1, __ATOMIC_SEQ_CST) != 0);
}

/**
 * Atomically computes var += delta.  It is legal for delta to be negative.
 * Returns the result of the addition.
 */
INLINE AtomicAdjustGccImpl::Integer AtomicAdjustGccImpl::
add(TVOLATILE AtomicAdjustGccImpl::Integer &var,
    AtomicAdjustGccImpl::Integer delta) {
  return __atomic_add_fetch(&var, delta, __ATOMIC_SEQ_CST);
}

/**
 * Atomically changes the indicated variable and returns the original value.
 */
INLINE AtomicAdjustGccImpl::Integer AtomicAdjustGccImpl::
set(TVOLATILE AtomicAdjustGccImpl::Integer &var,
    AtomicAdjustGccImpl::Integer new_value) {

  return __atomic_exchange_n(&var, new_value, __ATOMIC_SEQ_CST);
}

/**
 * Atomically retrieves the snapshot value of the indicated variable.  This is
 * the only guaranteed safe way to retrieve the value that other threads might
 * be asynchronously setting, incrementing, or decrementing (via other
 * AtomicAjust methods).
 */
INLINE AtomicAdjustGccImpl::Integer AtomicAdjustGccImpl::
get(const TVOLATILE AtomicAdjustGccImpl::Integer &var) {
  return __atomic_load_n(&var, __ATOMIC_SEQ_CST);
}

/**
 * Atomically changes the indicated variable and returns the original value.
 */
INLINE AtomicAdjustGccImpl::Pointer AtomicAdjustGccImpl::
set_ptr(TVOLATILE AtomicAdjustGccImpl::Pointer &var,
        AtomicAdjustGccImpl::Pointer new_value) {

  return __atomic_exchange_n(&var, new_value, __ATOMIC_SEQ_CST);
}

/**
 * Atomically retrieves the snapshot value of the indicated variable.  This is
 * the only guaranteed safe way to retrieve the value that other threads might
 * be asynchronously setting, incrementing, or decrementing (via other
 * AtomicAjust methods).
 */
INLINE AtomicAdjustGccImpl::Pointer AtomicAdjustGccImpl::
get_ptr(const TVOLATILE AtomicAdjustGccImpl::Pointer &var) {
  return __atomic_load_n(&var, __ATOMIC_SEQ_CST);
}

/**
 * Atomic compare and exchange.
 *
 * If mem is equal to old_value, store new_value in mem.  In either case,
 * return the original value of mem.  The caller can test for success by
 * comparing return_value == old_value.
 *
 * The atomic function expressed in pseudo-code:
 *
 * orig_value = mem; if (mem == old_value) { mem = new_value; } return
 * orig_value;
 *
 */
INLINE AtomicAdjustGccImpl::Integer AtomicAdjustGccImpl::
compare_and_exchange(TVOLATILE AtomicAdjustGccImpl::Integer &mem,
                     AtomicAdjustGccImpl::Integer old_value,
                     AtomicAdjustGccImpl::Integer new_value) {

  __atomic_compare_exchange_n(&mem, &old_value, new_value, false,
                              __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
  return old_value;
}

/**
 * Atomic compare and exchange.
 *
 * As above, but works on pointers instead of integers.
 */
INLINE AtomicAdjustGccImpl::Pointer AtomicAdjustGccImpl::
compare_and_exchange_ptr(TVOLATILE AtomicAdjustGccImpl::Pointer &mem,
                         AtomicAdjustGccImpl::Pointer old_value,
                         AtomicAdjustGccImpl::Pointer new_value) {

  __atomic_compare_exchange_n(&mem, &old_value, new_value, false,
                              __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
  return old_value;
}
