/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file patomic.cxx
 * @author rdb
 * @date 2022-02-23
 */

#include "patomic.h"

#include <functional>

static_assert(sizeof(patomic_unsigned_lock_free) == sizeof(uint32_t),
              "expected atomic uint32_t to have same size as uint32_t");
static_assert(sizeof(patomic_signed_lock_free) == sizeof(int32_t),
              "expected atomic int32_t to have same size as int32_t");
static_assert(sizeof(uint32_t) == sizeof(int32_t),
              "expected int32_t to have same size as uint32_t");

#if !defined(CPPPARSER) && defined(_WIN32)

// On Windows 7, we try to load the Windows 8 functions dynamically, and
// fall back to a condition variable table if they aren't available.
static BOOL __stdcall initialize_wait(volatile VOID *addr, PVOID cmp, SIZE_T size, DWORD timeout);
static void __stdcall dummy_wake(PVOID addr) {}

BOOL (__stdcall *_patomic_wait_func)(volatile VOID *, PVOID, SIZE_T, DWORD) = &initialize_wait;
void (__stdcall *_patomic_wake_one_func)(PVOID) = &dummy_wake;
void (__stdcall *_patomic_wake_all_func)(PVOID) = &dummy_wake;

// Randomly pick an entry into the wait table based on the hash of the address.
// It's possible to get hash collision, but that's not so bad, it just means
// that the other thread will get a spurious wakeup.
struct alignas(64) WaitTableEntry {
  SRWLOCK _lock = SRWLOCK_INIT;
  CONDITION_VARIABLE _cvar = CONDITION_VARIABLE_INIT;
  DWORD _waiters = 0;
};
static WaitTableEntry _wait_table[64] = {};
static const size_t _wait_hash_mask = 63;

/**
 * Emulates WakeByAddressSingle for Windows Vista and 7.
 */
static void __stdcall
emulated_wake(PVOID addr) {
  size_t i = std::hash<volatile void *>{}(addr) & (sizeof(_wait_table) / sizeof(WaitTableEntry) - 1);
  WaitTableEntry &entry = _wait_table[i];
  AcquireSRWLockExclusive(&entry._lock);
  DWORD num_waiters = entry._waiters;
  ReleaseSRWLockExclusive(&entry._lock);
  if (num_waiters > 0) {
    // We have to wake up all the threads, even if only one of them is for this
    // address.  Some of them will get a spurious wakeup, but that's OK.
    WakeAllConditionVariable(&entry._cvar);
  }
}

/**
 * Emulates WaitOnAddress for Windows Vista and 7.  Only supports aligned
 * 32-bit values.
 */
static BOOL __stdcall
emulated_wait(volatile VOID *addr, PVOID cmp, SIZE_T size, DWORD timeout) {
  assert(size == sizeof(LONG));

  LONG cmpval = *(LONG *)cmp;
  if (*(LONG *)addr != cmpval) {
    return TRUE;
  }

  size_t i = std::hash<volatile void *>{}(addr) & _wait_hash_mask;
  WaitTableEntry &entry = _wait_table[i];
  AcquireSRWLockExclusive(&entry._lock);
  ++entry._waiters;
  while (*(LONG *)addr == cmpval) {
    if (SleepConditionVariableSRW(&entry._cvar, &entry._lock, timeout, 0) != 0) {
      // Timeout.
      --entry._waiters;
      ReleaseSRWLockExclusive(&entry._lock);
      return FALSE;
    }
  }
  --entry._waiters;
  ReleaseSRWLockExclusive(&entry._lock);
  return TRUE;
}

/**
 * Initially assigned to the wait function slot to initialize the function
 * pointers.
 */
static BOOL __stdcall
initialize_wait(volatile VOID *addr, PVOID cmp, SIZE_T size, DWORD timeout) {
  // There's a chance of a race here, with two threads trying to initialize the
  // functions at the same time.  That's OK, because they should all produce
  // the same results, and the stores to the function pointers are atomic.
  HMODULE lib = GetModuleHandleW(L"api-ms-win-core-synch-l1-2-0.dll");
  if (lib) {
    auto wait_func = (decltype(_patomic_wait_func))GetProcAddress(lib, "WaitOnAddress");
    auto wake_one_func = (decltype(_patomic_wake_one_func))GetProcAddress(lib, "WakeByAddressSingle");
    auto wake_all_func = (decltype(_patomic_wake_all_func))GetProcAddress(lib, "WakeByAddressAll");
    if (wait_func && wake_one_func && wake_all_func) {
      // Make sure that the wake function is guaranteed to be visible to other
      // threads by the time we assign the wait function.
      _patomic_wake_one_func = wake_one_func;
      _patomic_wake_all_func = wake_all_func;
      patomic_thread_fence(std::memory_order_release);
      _patomic_wait_func = wait_func;
      return wait_func(addr, cmp, size, timeout);
    }
  }

  // We don't have Windows 8's functions, use the emulated wait and wake funcs.
  _patomic_wake_one_func = &emulated_wake;
  _patomic_wake_all_func = &emulated_wake;
  patomic_thread_fence(std::memory_order_release);
  _patomic_wait_func = &emulated_wait;

  return emulated_wait(addr, cmp, size, timeout);
}

#elif !defined(CPPPARSER) && !defined(__linux__) && !defined(__APPLE__) && defined(HAVE_POSIX_THREADS)

// Same as above, but using pthreads.
struct alignas(64) WaitTableEntry {
  pthread_mutex_t _lock = PTHREAD_MUTEX_INITIALIZER;
  pthread_cond_t _cvar = PTHREAD_COND_INITIALIZER;
  unsigned int _waiters = 0;
};
static WaitTableEntry _wait_table[64];
static const size_t _wait_hash_mask = 63;

/**
 *
 */
void
_patomic_wait(const volatile uint32_t *value, uint32_t old) {
  WaitTableEntry &entry = _wait_table[std::hash<const volatile void *>{}(value) & _wait_hash_mask];
  pthread_mutex_lock(&entry._lock);
  ++entry._waiters;
  while (__atomic_load_n(value, __ATOMIC_SEQ_CST) == old) {
    pthread_cond_wait(&entry._cvar, &entry._lock);
  }
  --entry._waiters;
  pthread_mutex_unlock(&entry._lock);
}

/**
 *
 */
void
_patomic_notify_all(volatile uint32_t *value) {
  WaitTableEntry &entry = _wait_table[std::hash<const volatile void *>{}(value) & _wait_hash_mask];
  pthread_mutex_lock(&entry._lock);
  unsigned int num_waiters = entry._waiters;
  pthread_mutex_unlock(&entry._lock);
  if (num_waiters > 0) {
    pthread_cond_broadcast(&entry._cvar);
  }
}

#endif
