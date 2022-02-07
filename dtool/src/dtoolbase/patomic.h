/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file patomic.h
 * @author rdb
 * @date 2022-01-28
 */

#ifndef PATOMIC_H
#define PATOMIC_H

#include "dtoolbase.h"
#include "selectThreadImpl.h"

#include <atomic>

#if defined(THREAD_DUMMY_IMPL) || defined(THREAD_SIMPLE_IMPL)

/**
 * Dummy implementation of std::atomic that does not do any atomic operations,
 * used when compiling without HAVE_THREADS or with SIMPLE_THREADS.
 */
template<class T>
struct patomic {
  using value_type = T;

  constexpr patomic() noexcept = default;
  constexpr patomic(T desired) noexcept;

  ALWAYS_INLINE patomic(const patomic &) = delete;
  ALWAYS_INLINE patomic &operator=(const patomic &) = delete;

  static constexpr bool is_always_lock_free = true;
  ALWAYS_INLINE bool is_lock_free() const noexcept;

  ALWAYS_INLINE T load(std::memory_order order = std::memory_order_seq_cst) const noexcept;
  ALWAYS_INLINE operator T() const noexcept;

  ALWAYS_INLINE void store(T desired, std::memory_order order = std::memory_order_seq_cst) noexcept;
  ALWAYS_INLINE T operator=(T desired) noexcept;

  ALWAYS_INLINE T exchange(T desired, std::memory_order order = std::memory_order_seq_cst) noexcept;

  ALWAYS_INLINE bool compare_exchange_weak(T &expected, T desired,
                                           std::memory_order success = std::memory_order_seq_cst,
                                           std::memory_order failure = std::memory_order_seq_cst) noexcept;
  ALWAYS_INLINE bool compare_exchange_strong(T &expected, T desired,
                                             std::memory_order success = std::memory_order_seq_cst,
                                             std::memory_order failure = std::memory_order_seq_cst) noexcept;

  ALWAYS_INLINE T fetch_add(T arg, std::memory_order order = std::memory_order_seq_cst) noexcept;
  ALWAYS_INLINE T fetch_sub(T arg, std::memory_order order = std::memory_order_seq_cst) noexcept;
  ALWAYS_INLINE T fetch_and(T arg, std::memory_order order = std::memory_order_seq_cst) noexcept;
  ALWAYS_INLINE T fetch_or(T arg, std::memory_order order = std::memory_order_seq_cst) noexcept;
  ALWAYS_INLINE T fetch_xor(T arg, std::memory_order order = std::memory_order_seq_cst) noexcept;

  ALWAYS_INLINE T operator ++(int) noexcept;
  ALWAYS_INLINE T operator --(int) noexcept;
  ALWAYS_INLINE T operator ++() noexcept;
  ALWAYS_INLINE T operator --() noexcept;
  ALWAYS_INLINE T operator +=(T arg) noexcept;
  ALWAYS_INLINE T operator -=(T arg) noexcept;
  ALWAYS_INLINE T operator &=(T arg) noexcept;
  ALWAYS_INLINE T operator |=(T arg) noexcept;
  ALWAYS_INLINE T operator ^=(T arg) noexcept;

private:
  T _value;
};

/**
 * Dummy implementation of std::atomic_flag that does not do any atomic
 * operations.
 */
struct EXPCL_DTOOL_DTOOLBASE patomic_flag {
  constexpr patomic_flag() noexcept = default;

  patomic_flag(const patomic_flag &) = delete;
  patomic_flag &operator=(const patomic_flag &) = delete;

  ALWAYS_INLINE bool test_and_set(std::memory_order order = std::memory_order_seq_cst) noexcept;
  ALWAYS_INLINE void clear(std::memory_order order = std::memory_order_seq_cst) noexcept;

  bool __internal_flag = false;
};

#define patomic_thread_fence(order) (std::atomic_signal_fence((order)))

#include "patomic.I"

#else

// We're using real threading, so use the real implementation.
template<class T>
using patomic = std::atomic<T>;

typedef std::atomic_flag patomic_flag;

#define patomic_thread_fence(order) (std::atomic_thread_fence((order)))

#endif

#endif
