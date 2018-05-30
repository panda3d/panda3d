/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file atomicAdjustWin32Impl.I
 * @author drose
 * @date 2006-02-07
 */

/**
 * Atomically increments the indicated variable.
 */
ALWAYS_INLINE void AtomicAdjustWin32Impl::
inc(TVOLATILE AtomicAdjustWin32Impl::Integer &var) {
  assert((((size_t)&var) & (sizeof(Integer) - 1)) == 0);
#ifdef _WIN64
  InterlockedIncrement64(&var);
#else
  InterlockedIncrement(&var);
#endif  // _WIN64
}

/**
 * Atomically decrements the indicated variable and returns true if the new
 * value is nonzero, false if it is zero.
 */
ALWAYS_INLINE bool AtomicAdjustWin32Impl::
dec(TVOLATILE AtomicAdjustWin32Impl::Integer &var) {
  assert((((size_t)&var) & (sizeof(Integer) - 1)) == 0);
#ifdef _WIN64
  return (InterlockedDecrement64(&var) != 0);
#else
  return (InterlockedDecrement(&var) != 0);
#endif  // _WIN64
}

/**
 * Atomically computes var += delta.  It is legal for delta to be negative.
 * Returns the result of the addition.
 */
INLINE AtomicAdjustWin32Impl::Integer AtomicAdjustWin32Impl::
add(TVOLATILE AtomicAdjustWin32Impl::Integer &var, AtomicAdjustWin32Impl::Integer delta) {
  assert((((size_t)&var) & (sizeof(Integer) - 1)) == 0);
#ifdef _WIN64
  return InterlockedAdd64(&var, delta);
#else
  Integer orig_value = var;
  Integer new_value = orig_value + delta;
  while (compare_and_exchange(var, orig_value, new_value) != orig_value) {
    orig_value = var;
    new_value = orig_value + delta;
  }
  return new_value;
#endif  // _WIN64
}

/**
 * Atomically changes the indicated variable and returns the original value.
 */
ALWAYS_INLINE AtomicAdjustWin32Impl::Integer AtomicAdjustWin32Impl::
set(TVOLATILE AtomicAdjustWin32Impl::Integer &var,
    AtomicAdjustWin32Impl::Integer new_value) {
  assert((((size_t)&var) & (sizeof(Integer) - 1)) == 0);
#ifdef _WIN64
  return InterlockedExchange64(&var, new_value);
#else
  return InterlockedExchange(&var, new_value);
#endif  // _WIN64
}

/**
 * Atomically retrieves the snapshot value of the indicated variable.  This is
 * the only guaranteed safe way to retrieve the value that other threads might
 * be asynchronously setting, incrementing, or decrementing (via other
 * AtomicAjust methods).
 */
ALWAYS_INLINE AtomicAdjustWin32Impl::Integer AtomicAdjustWin32Impl::
get(const TVOLATILE AtomicAdjustWin32Impl::Integer &var) {
  // On Intel platforms, word-aligned loads are atomic (if performed in a
  // single instruction).  We can't guarantee the compiler will generate a
  // single instruction to load this value, but it certainly won't happen if
  // its address isn't word-aligned, so make sure that's the case.
  assert((((size_t)&var) & (sizeof(Integer) - 1)) == 0);
  return var;
}

/**
 * Atomically changes the indicated variable and returns the original value.
 */
ALWAYS_INLINE AtomicAdjustWin32Impl::Pointer AtomicAdjustWin32Impl::
set_ptr(TVOLATILE AtomicAdjustWin32Impl::Pointer &var,
        AtomicAdjustWin32Impl::Pointer new_value) {
  assert((((size_t)&var) & (sizeof(Pointer) - 1)) == 0);
  return InterlockedExchangePointer(&var, new_value);
}

/**
 * Atomically retrieves the snapshot value of the indicated variable.  This is
 * the only guaranteed safe way to retrieve the value that other threads might
 * be asynchronously setting, incrementing, or decrementing (via other
 * AtomicAjust methods).
 */
ALWAYS_INLINE AtomicAdjustWin32Impl::Pointer AtomicAdjustWin32Impl::
get_ptr(const TVOLATILE AtomicAdjustWin32Impl::Pointer &var) {
  // As in get(), make sure the address is word-aligned.
  assert((((size_t)&var) & (sizeof(Pointer) - 1)) == 0);
  return var;
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
INLINE AtomicAdjustWin32Impl::Integer AtomicAdjustWin32Impl::
compare_and_exchange(TVOLATILE AtomicAdjustWin32Impl::Integer &mem,
                     AtomicAdjustWin32Impl::Integer old_value,
                     AtomicAdjustWin32Impl::Integer new_value) {
  assert((((size_t)&mem) & (sizeof(Integer) - 1)) == 0);
  // Note that the AtomicAdjust parameter order is different from Windows
  // convention!
#ifdef _WIN64
  return InterlockedCompareExchange64((TVOLATILE LONGLONG *)&mem, new_value, old_value);
#else
  return InterlockedCompareExchange((TVOLATILE LONG *)&mem, new_value, old_value);
#endif  // _WIN64
}

/**
 * Atomic compare and exchange.
 *
 * As above, but works on pointers instead of integers.
 */
INLINE AtomicAdjustWin32Impl::Pointer AtomicAdjustWin32Impl::
compare_and_exchange_ptr(TVOLATILE AtomicAdjustWin32Impl::Pointer &mem,
                         AtomicAdjustWin32Impl::Pointer old_value,
                         AtomicAdjustWin32Impl::Pointer new_value) {
  assert((((size_t)&mem) & (sizeof(Pointer) - 1)) == 0);
  // Note that the AtomicAdjust parameter order is different from Windows
  // convention!
  return InterlockedCompareExchangePointer(&mem, new_value, old_value);
}
