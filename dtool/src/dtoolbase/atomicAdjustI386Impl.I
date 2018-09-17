/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file atomicAdjustI386Impl.I
 * @author drose
 * @date 2006-04-01
 */

/**
 * Atomically increments the indicated variable.
 */
INLINE void AtomicAdjustI386Impl::
inc(TVOLATILE AtomicAdjustI386Impl::Integer &var) {
  assert((((size_t)&var) & (sizeof(Integer) - 1)) == 0);
#ifdef _M_IX86
  // Windows case
  TVOLATILE Integer *var_ptr = &var;
  __asm {
    mov edx, var_ptr;
    lock inc dword ptr [edx];
  }
#elif !defined(__EDG__)
  // GCC case
  __asm__ __volatile__("lock; incl %0"
                       :"=m" (var)
                       :"m" (&var));
#endif  // __EDG__
}

/**
 * Atomically decrements the indicated variable and returns true if the new
 * value is nonzero, false if it is zero.
 */
INLINE bool AtomicAdjustI386Impl::
dec(TVOLATILE AtomicAdjustI386Impl::Integer &var) {
  assert((((size_t)&var) & (sizeof(Integer) - 1)) == 0);
  unsigned char c;
#ifdef _M_IX86
  // Windows case
  TVOLATILE Integer *var_ptr = &var;
  __asm {
    mov edx, var_ptr;
    lock dec dword ptr [edx];
    sete c;
  }
#elif !defined(__EDG__)
  // GCC case
  __asm__ __volatile__("lock; decl %0; sete %1"
                       :"=m" (var), "=qm" (c)
                       :"m" (&var) : "memory");
#endif  // __EDG__
  return (c == 0);
}

/**
 * Atomically computes var += delta.  It is legal for delta to be negative.
 * Returns the result of the addition.
 */
INLINE AtomicAdjustI386Impl::Integer AtomicAdjustI386Impl::
add(TVOLATILE AtomicAdjustI386Impl::Integer &var, AtomicAdjustI386Impl::Integer delta) {
  assert((((size_t)&var) & (sizeof(Integer) - 1)) == 0);
  Integer orig_value = var;
  Integer new_value = orig_value + delta;
  while (compare_and_exchange(var, orig_value, new_value) != orig_value) {
    orig_value = var;
    new_value = orig_value + delta;
  }
  return new_value;
}

/**
 * Atomically changes the indicated variable and returns the original value.
 */
INLINE AtomicAdjustI386Impl::Integer AtomicAdjustI386Impl::
set(TVOLATILE AtomicAdjustI386Impl::Integer &var,
    AtomicAdjustI386Impl::Integer new_value) {
  assert((((size_t)&var) & (sizeof(Integer) - 1)) == 0);
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
INLINE AtomicAdjustI386Impl::Integer AtomicAdjustI386Impl::
get(const TVOLATILE AtomicAdjustI386Impl::Integer &var) {
  assert((((size_t)&var) & (sizeof(Integer) - 1)) == 0);
  return var;
}

/**
 * Atomically changes the indicated variable and returns the original value.
 */
INLINE AtomicAdjustI386Impl::Pointer AtomicAdjustI386Impl::
set_ptr(TVOLATILE AtomicAdjustI386Impl::Pointer &var,
        AtomicAdjustI386Impl::Pointer new_value) {
  assert((((size_t)&var) & (sizeof(Pointer) - 1)) == 0);
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
INLINE AtomicAdjustI386Impl::Pointer AtomicAdjustI386Impl::
get_ptr(const TVOLATILE AtomicAdjustI386Impl::Pointer &var) {
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
INLINE AtomicAdjustI386Impl::Integer AtomicAdjustI386Impl::
compare_and_exchange(TVOLATILE AtomicAdjustI386Impl::Integer &mem,
                     AtomicAdjustI386Impl::Integer old_value,
                     AtomicAdjustI386Impl::Integer new_value) {
  assert((((size_t)&mem) & (sizeof(Integer) - 1)) == 0);
  Integer prev;
#ifdef _M_IX86
  // Windows case
  TVOLATILE Integer *mem_ptr = &mem;
  __asm {
    mov edx, mem_ptr;
    mov ecx, new_value;
    mov eax, old_value;
    lock cmpxchg dword ptr [edx], ecx;
    mov prev, eax;
  }
#elif !defined(__EDG__)
  // GCC case
  __asm__ __volatile__("lock; cmpxchgl %1,%2"
                       : "=a"(prev)
                       : "r"(new_value), "m"(mem), "0"(old_value)
                       : "memory");
#endif  // __EDG__
  return prev;
}

/**
 * Atomic compare and exchange.
 *
 * As above, but works on pointers instead of integers.
 */
INLINE AtomicAdjustI386Impl::Pointer AtomicAdjustI386Impl::
compare_and_exchange_ptr(TVOLATILE AtomicAdjustI386Impl::Pointer &mem,
                         AtomicAdjustI386Impl::Pointer old_value,
                         AtomicAdjustI386Impl::Pointer new_value) {
  assert((((size_t)&mem) & (sizeof(Pointer) - 1)) == 0);
  Pointer prev;
#ifdef _M_IX86
  // Windows case
  TVOLATILE Pointer *mem_ptr = &mem;
  __asm {
    mov edx, mem_ptr;
    mov ecx, new_value;
    mov eax, old_value;
    lock cmpxchg dword ptr [edx], ecx;
    mov prev, eax;
  }
#elif !defined(__EDG__)
  // GCC case
  __asm__ __volatile__("lock; cmpxchgl %1,%2"
                       : "=a"(prev)
                       : "r"(new_value), "m"(mem), "0"(old_value)
                       : "memory");
#endif  // __EDG__
  return prev;
}
