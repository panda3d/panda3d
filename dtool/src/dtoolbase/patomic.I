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


/**
 * Sets the flag to true and returns the previous value.
 */
ALWAYS_INLINE bool patomic_flag::
test_and_set(std::memory_order order) noexcept {
  bool value = __internal_flag;
  __internal_flag = true;
  return value;
}

/**
 * Sets the flag to false.
 */
ALWAYS_INLINE void patomic_flag::
clear(std::memory_order order) noexcept {
  __internal_flag = false;
}
