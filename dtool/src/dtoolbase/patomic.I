/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file patomic.I
 * @author rdb
 * @date 2022-01-28
 */

#if defined(THREAD_DUMMY_IMPL) || defined(THREAD_SIMPLE_IMPL)
/**
 * Value initializer.
 */
template<class T>
constexpr patomic<T>::
patomic(T desired) noexcept : _value(desired) {
}

/**
 * Returns true if this is a lock free type (which it always is).
 */
template<class T>
ALWAYS_INLINE bool patomic<T>::
is_lock_free() const noexcept {
  return true;
}

/**
 * Returns the stored value.
 */
template<class T>
ALWAYS_INLINE T patomic<T>::
load(std::memory_order order) const noexcept {
  return _value;
}

/**
 * Returns the stored value.
 */
template<class T>
ALWAYS_INLINE patomic<T>::
operator T() const noexcept {
  return _value;
}

/**
 * Changes the stored value.
 */
template<class T>
ALWAYS_INLINE void patomic<T>::
store(T desired, std::memory_order order) noexcept {
  _value = desired;
}

/**
 * Changes the stored value.
 */
template<class T>
ALWAYS_INLINE T patomic<T>::
operator=(T desired) noexcept {
  _value = desired;
  return desired;
}

/**
 * Changes the stored value, returning the previous value.
 */
template<class T>
ALWAYS_INLINE T patomic<T>::
exchange(T desired, std::memory_order) noexcept {
  T current = _value;
  _value = desired;
  return current;
}

/**
 * Sets the desired value if the current value is as the first argument.
 * If it is not, the current value is written to expected.
 */
template<class T>
ALWAYS_INLINE bool patomic<T>::
compare_exchange_weak(T &expected, T desired,
                      std::memory_order, std::memory_order) noexcept {
  T current = _value;
  if (_value == expected) {
    _value = desired;
    return true;
  } else {
    expected = current;
    return false;
  }
}

/**
 * Sets the desired value if the current value is as the first argument.
 * If it is not, the current value is written to expected.
 */
template<class T>
ALWAYS_INLINE bool patomic<T>::
compare_exchange_strong(T &expected, T desired,
                        std::memory_order, std::memory_order) noexcept {
  T current = _value;
  if (_value == expected) {
    _value = desired;
    return true;
  } else {
    expected = current;
    return false;
  }
}

/**
 * Adds to the stored value, returns the old value.
 */
template<class T>
ALWAYS_INLINE T patomic<T>::
fetch_add(T arg, std::memory_order) noexcept {
  T old = _value;
  _value += arg;
  return old;
}

/**
 *
 */
template<class T>
ALWAYS_INLINE T patomic<T>::
fetch_sub(T arg, std::memory_order) noexcept {
  T old = _value;
  _value -= arg;
  return old;
}

/**
 *
 */
template<class T>
ALWAYS_INLINE T patomic<T>::
fetch_and(T arg, std::memory_order) noexcept {
  T old = _value;
  _value &= arg;
  return old;
}

/**
 *
 */
template<class T>
ALWAYS_INLINE T patomic<T>::
fetch_or(T arg, std::memory_order) noexcept {
  T old = _value;
  _value |= arg;
  return old;
}

/**
 *
 */
template<class T>
ALWAYS_INLINE T patomic<T>::
fetch_xor(T arg, std::memory_order) noexcept {
  T old = _value;
  _value ^= arg;
  return old;
}

/**
 *
 */
template<class T>
ALWAYS_INLINE T patomic<T>::
operator ++(int) noexcept {
  return _value++;
}

/**
 *
 */
template<class T>
ALWAYS_INLINE T patomic<T>::
operator --(int) noexcept {
  return _value--;
}

/**
 *
 */
template<class T>
ALWAYS_INLINE T patomic<T>::
operator ++() noexcept {
  return ++_value;
}

/**
 *
 */
template<class T>
ALWAYS_INLINE T patomic<T>::
operator --() noexcept {
  return --_value;
}

/**
 *
 */
template<class T>
ALWAYS_INLINE T patomic<T>::
operator +=(T arg) noexcept {
  return _value += arg;
}

/**
 *
 */
template<class T>
ALWAYS_INLINE T patomic<T>::
operator -=(T arg) noexcept {
  return _value -= arg;
}

/**
 *
 */
template<class T>
ALWAYS_INLINE T patomic<T>::
operator &=(T arg) noexcept {
  return _value &= arg;
}

/**
 *
 */
template<class T>
ALWAYS_INLINE T patomic<T>::
operator |=(T arg) noexcept {
  return _value |= arg;
}

/**
 *
 */
template<class T>
ALWAYS_INLINE T patomic<T>::
operator ^=(T arg) noexcept {
  return _value ^= arg;
}

#endif  // defined(THREAD_DUMMY_IMPL) || defined(THREAD_SIMPLE_IMPL)

/**
 * Initializes the variable to zero (according to C++20 semantics, NOT C++11
 * semantics!)
 */
constexpr patomic_unsigned_lock_free::
patomic_unsigned_lock_free() noexcept :
  patomic<uint32_t>(0u) {
}

/**
 * Initializes the variable to the given value.
 */
constexpr patomic_unsigned_lock_free::
patomic_unsigned_lock_free(uint32_t desired) noexcept :
  patomic<uint32_t>(desired) {
}

/**
 * Waits until the value is no longer equal to the given value.
 */
ALWAYS_INLINE void patomic_unsigned_lock_free::
wait(uint32_t old, std::memory_order order) const noexcept {
  if (load(order) == old) {
    patomic_wait((const volatile uint32_t *)this, old);
  }
}

/**
 * Wakes up at least one thread waiting for the value to change.
 */
ALWAYS_INLINE void patomic_unsigned_lock_free::
notify_one() noexcept {
  patomic_notify_one((volatile uint32_t *)this);
}

/**
 * Wakes up at least one thread waiting for the value to change.
 */
ALWAYS_INLINE void patomic_unsigned_lock_free::
notify_all() noexcept {
  patomic_notify_all((volatile uint32_t *)this);
}

/**
 * Initializes the variable to zero (according to C++20 semantics, NOT C++11
 * semantics!)
 */
constexpr patomic_signed_lock_free::
patomic_signed_lock_free() noexcept :
  patomic<int32_t>(0) {
}

/**
 * Initializes the variable to the given value.
 */
constexpr patomic_signed_lock_free::
patomic_signed_lock_free(int32_t desired) noexcept :
  patomic<int32_t>(desired) {
}

/**
 * Waits until the value is no longer equal to the given value.
 */
ALWAYS_INLINE void patomic_signed_lock_free::
wait(int32_t old, std::memory_order order) const noexcept {
  if (load(order) == old) {
    patomic_wait((const volatile int32_t *)this, old);
  }
}

/**
 * Wakes up at least one thread waiting for the value to change.
 */
ALWAYS_INLINE void patomic_signed_lock_free::
notify_one() noexcept {
  patomic_notify_one((volatile int32_t *)this);
}

/**
 * Wakes up at least one thread waiting for the value to change.
 */
ALWAYS_INLINE void patomic_signed_lock_free::
notify_all() noexcept {
  patomic_notify_all((volatile int32_t *)this);
}

/**
 * Allows assignment from ATOMIC_FLAG_INIT.
 */
constexpr patomic_flag::
patomic_flag(bool desired) noexcept : _value(desired) {
}

/**
 * Sets the flag to false.
 */
ALWAYS_INLINE void patomic_flag::
clear(std::memory_order order) noexcept {
  _value.store(0u, order);
}

/**
 * Sets the flag to true and returns the previous value.
 */
ALWAYS_INLINE bool patomic_flag::
test_and_set(std::memory_order order) noexcept {
  return _value.exchange(1u, order) != 0u;
}

/**
 * Returns the current value of the flag.
 */
ALWAYS_INLINE bool patomic_flag::
test(std::memory_order order) const noexcept {
  return _value.load(order) != 0u;
}

/**
 * Waits until the value is no longer equal to the given value.
 */
ALWAYS_INLINE void patomic_flag::
wait(bool old, std::memory_order order) const noexcept {
  _value.wait(old, order);
}

/**
 * Wakes up at least one thread waiting for the value to change.
 */
ALWAYS_INLINE void patomic_flag::
notify_one() noexcept {
  _value.notify_one();
}

/**
 * Wakes up at least one thread waiting for the value to change.
 */
ALWAYS_INLINE void patomic_flag::
notify_all() noexcept {
  _value.notify_all();
}

/**
 *
 */
ALWAYS_INLINE void
patomic_wait(const volatile int32_t *value, int32_t old) {
  patomic_wait((const volatile uint32_t *)value, (uint32_t)old);
}

/**
 *
 */
ALWAYS_INLINE void
patomic_notify_one(volatile int32_t *value) {
  patomic_notify_one((volatile uint32_t *)value);
}

/**
 *
 */
ALWAYS_INLINE void
patomic_notify_all(volatile int32_t *value) {
  patomic_notify_all((volatile uint32_t *)value);
}

/**
 *
 */
ALWAYS_INLINE void
patomic_wait(const volatile uint32_t *value, uint32_t old) {
#ifdef __linux__
  while (__atomic_load_n(value, __ATOMIC_SEQ_CST) == old) {
    syscall(SYS_futex, old, FUTEX_WAIT_PRIVATE, old, 0, 0, 0);
  }
//#elif _WIN32_WINNT >= _WIN32_WINNT_WIN8
//  while (*value == old) {
//    WaitOnAddress((volatile void *)value, &old, sizeof(uint32_t), INFINITE);
//  }
#elif defined(_WIN32)
  while (*value == old) {
    _patomic_wait_func((volatile void *)value, &old, sizeof(uint32_t), INFINITE);
  }
#elif defined(__APPLE__)
  __ulock_wait(UL_COMPARE_AND_WAIT, (void *)value, old, 0);
#elif defined(HAVE_POSIX_THREADS)
  _patomic_wait(value, old);
#else
  while (*value == old);
#endif
}

/**
 *
 */
ALWAYS_INLINE void
patomic_notify_one(volatile uint32_t *value) {
#ifdef __linux__
  syscall(SYS_futex, value, FUTEX_WAKE_PRIVATE, 1, 0, 0, 0);
//#elif _WIN32_WINNT >= _WIN32_WINNT_WIN8
//  WakeByAddressSingle((void *)value);
#elif defined(_WIN32)
  _patomic_wake_one_func((void *)value);
#elif defined(__APPLE__)
  __ulock_wake(UL_COMPARE_AND_WAIT, (void *)value, 0);
#elif defined(HAVE_POSIX_THREADS)
  _patomic_notify_all(value);
#endif
}

/**
 *
 */
ALWAYS_INLINE void
patomic_notify_all(volatile uint32_t *value) {
#ifdef __linux__
  syscall(SYS_futex, value, FUTEX_WAKE_PRIVATE, INT_MAX, 0, 0, 0);
//#elif _WIN32_WINNT >= _WIN32_WINNT_WIN8
//  WakeByAddressAll((void *)value);
#elif defined(_WIN32)
  _patomic_wake_all_func((void *)value);
#elif defined(__APPLE__)
  __ulock_wake(UL_COMPARE_AND_WAIT | ULF_WAKE_ALL, (void *)value, 0);
#elif defined(HAVE_POSIX_THREADS)
  _patomic_notify_all(value);
#endif
}
