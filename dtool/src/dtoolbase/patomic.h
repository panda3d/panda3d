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

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
#endif

#ifdef __linux__
#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>
#endif

#ifdef __APPLE__
// Undocumented API, see https://outerproduct.net/futex-dictionary.html
#define UL_COMPARE_AND_WAIT 1
#define ULF_WAKE_ALL 0x00000100

extern "C" int __ulock_wait(uint32_t op, void *addr, uint64_t value, uint32_t timeout);
extern "C" int __ulock_wake(uint32_t op, void *addr, uint64_t wake_value);
#endif

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

#define patomic_thread_fence(order) (std::atomic_signal_fence((order)))

#else

// We're using real threading, so use the real implementation.
template<class T>
using patomic = std::atomic<T>;

#define patomic_thread_fence(order) (std::atomic_thread_fence((order)))

#endif

/**
 * Implementation of atomic_unsigned_lock_free with C++20 semantics.
 */
class EXPCL_DTOOL_DTOOLBASE patomic_unsigned_lock_free : public patomic<uint32_t> {
public:
  typedef uint32_t value_type;

  constexpr patomic_unsigned_lock_free() noexcept;
  constexpr patomic_unsigned_lock_free(uint32_t desired) noexcept;

  INLINE void wait(uint32_t old, std::memory_order order = std::memory_order_seq_cst) const noexcept;
  ALWAYS_INLINE void notify_one() noexcept;
  ALWAYS_INLINE void notify_all() noexcept;
};

/**
 * Implementation of atomic_signed_lock_free with C++20 semantics.
 */
class EXPCL_DTOOL_DTOOLBASE patomic_signed_lock_free : public patomic<int32_t> {
public:
  typedef int32_t value_type;

  constexpr patomic_signed_lock_free() noexcept;
  constexpr patomic_signed_lock_free(int32_t desired) noexcept;

  INLINE void wait(int32_t old, std::memory_order order = std::memory_order_seq_cst) const noexcept;
  ALWAYS_INLINE void notify_one() noexcept;
  ALWAYS_INLINE void notify_all() noexcept;
};

/**
 * Implementation of atomic_flag with C++20 semantics.
 */
class EXPCL_DTOOL_DTOOLBASE patomic_flag {
public:
  constexpr patomic_flag() noexcept = default;
  constexpr patomic_flag(bool desired) noexcept;

  patomic_flag(const patomic_flag &) = delete;
  patomic_flag &operator=(const patomic_flag &) = delete;

  ALWAYS_INLINE void clear(std::memory_order order = std::memory_order_seq_cst) noexcept;
  ALWAYS_INLINE bool test_and_set(std::memory_order order = std::memory_order_seq_cst) noexcept;
  ALWAYS_INLINE bool test(std::memory_order order = std::memory_order_seq_cst) const noexcept;

  ALWAYS_INLINE void wait(bool old, std::memory_order order = std::memory_order_seq_cst) const noexcept;
  ALWAYS_INLINE void notify_one() noexcept;
  ALWAYS_INLINE void notify_all() noexcept;

private:
  patomic_unsigned_lock_free _value { 0u };
};

#ifndef CPPPARSER
ALWAYS_INLINE void patomic_wait(const volatile int32_t *value, int32_t old);
ALWAYS_INLINE void patomic_notify_one(volatile int32_t *value);
ALWAYS_INLINE void patomic_notify_all(volatile int32_t *value);

ALWAYS_INLINE void patomic_wait(const volatile uint32_t *value, uint32_t old);
ALWAYS_INLINE void patomic_notify_one(volatile uint32_t *value);
ALWAYS_INLINE void patomic_notify_all(volatile uint32_t *value);

#ifdef _WIN32
EXPCL_DTOOL_DTOOLBASE extern BOOL (__stdcall *_patomic_wait_func)(volatile VOID *, PVOID, SIZE_T, DWORD);
EXPCL_DTOOL_DTOOLBASE extern void (__stdcall *_patomic_wake_one_func)(PVOID);
EXPCL_DTOOL_DTOOLBASE extern void (__stdcall *_patomic_wake_all_func)(PVOID);
#elif !defined(__linux__) && !defined(__APPLE__) && defined(HAVE_POSIX_THREADS)
EXPCL_DTOOL_DTOOLBASE void _patomic_wait(const volatile uint32_t *value, uint32_t old);
EXPCL_DTOOL_DTOOLBASE void _patomic_notify_all(volatile uint32_t *value);
#endif

#include "patomic.I"
#endif  // CPPPARSER

#endif
