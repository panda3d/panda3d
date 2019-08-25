/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mutexWin32Impl.h
 * @author drose
 * @date 2006-02-07
 */

#ifndef MUTEXWIN32IMPL_H
#define MUTEXWIN32IMPL_H

#include "dtoolbase.h"
#include "selectThreadImpl.h"

#ifdef WIN32_VC
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>

/**
 * Uses SRWLock on Vista and above to implement a mutex.  Older versions of
 * Windows fall back to a combination of a spinlock and an Event object.
 */
class EXPCL_DTOOL_DTOOLBASE MutexWin32Impl {
public:
  constexpr MutexWin32Impl() = default;
  MutexWin32Impl(const MutexWin32Impl &copy) = delete;
  INLINE ~MutexWin32Impl();

  MutexWin32Impl &operator = (const MutexWin32Impl &copy) = delete;

public:
  INLINE void lock();
  INLINE bool try_lock();
  INLINE void unlock();

  static void init_lock_funcs();

private:
#ifndef CPPPARSER
  // Store function pointers; these point directly to the SRWLock Win32 API
  // functions on Vista and above, or to our own implementation on Windows XP.
  typedef void (__stdcall *LockFunc)(volatile PVOID *lock);
  typedef BOOL (__stdcall *TryLockFunc)(volatile PVOID *lock);
  typedef BOOL (__stdcall *CondWaitFunc)(volatile PVOID *cvar, volatile PVOID *lock, DWORD, ULONG);

  struct LockFunctions {
    LockFunc _lock;
    TryLockFunc _try_lock;
    LockFunc _unlock;

    CondWaitFunc _cvar_wait;
    LockFunc _cvar_notify_one;
    LockFunc _cvar_notify_all;
  };

  static LockFunctions _funcs;

  static void __stdcall lock_initially(volatile PVOID *lock);
  static BOOL __stdcall try_lock_initially(volatile PVOID *lock);
  static void __stdcall unlock_initially(volatile PVOID *lock);
#endif

private:
  // In the SRWLock implementation, only the first field is used.  On Windows
  // XP, the first field contains a waiter count and lock bit, and the second
  // field contains an Event handle if contention has occurred.
  volatile PVOID _lock[2] = {nullptr, nullptr};

  friend class ConditionVarWin32Impl;
};

/**
 * Uses CRITICAL_SECTION to implement a recursive mutex.
 */
class EXPCL_DTOOL_DTOOLBASE ReMutexWin32Impl {
public:
  ReMutexWin32Impl();
  ReMutexWin32Impl(const ReMutexWin32Impl &copy) = delete;
  INLINE ~ReMutexWin32Impl();

  ReMutexWin32Impl &operator = (const ReMutexWin32Impl &copy) = delete;

public:
  INLINE void lock();
  INLINE bool try_lock();
  INLINE void unlock();

private:
  CRITICAL_SECTION _lock;
};

#include "mutexWin32Impl.I"

#endif  // WIN32_VC

#endif
