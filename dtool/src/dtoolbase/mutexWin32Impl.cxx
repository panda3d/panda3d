/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mutexWin32Impl.cxx
 * @author drose
 * @date 2006-02-07
 */

#include "selectThreadImpl.h"

#if defined(WIN32_VC) && !defined(CPPPARSER)

#include "mutexWin32Impl.h"

// The number of spins to do before suspending the thread.
static const unsigned int spin_count = 4000;

// Only compile the below nonsense if we're not compiling for a Vista minimum.
#if _WIN32_WINNT < 0x0600

// If this is true, we will use SRWLock on Windows Vista and above instead of
// our own implementation.
static const bool prefer_srwlock = true;

// These configure our own Windows XP implementation.
static const uintptr_t lock_bit = 0x40000000;

// This gets set to spin_count if we are on a multi-core system.
static unsigned int effective_spin_count = 0;

/**
 * Windows XP implementation of lock(), which uses a combination of a spinlock
 * and an event.
 */
static void __stdcall lock_xp(volatile PVOID *lock) {
  // In the Windows XP case, lock consists of two words: the first one is a
  // number of waiters plus a bit to indicate that it is locked, the second
  // one is the handle of an event that is created in case of contention.
  // The first word can be in the following states:
  //
  // lock bit | waiters | meaning
  // ---------|---------|---------
  //   unset  |    0    | unlocked
  //     set  |    0    | locked, nobody waiting on event
  //     set  |   >0    | locked, at least one thread waiting on event
  //   unset  |   >0    | handing off lock to one of waiters
  //
  // The last state is a little subtle: at this point, the thread that was
  // holding the lock has stopped holding it, but is about to fire off a
  // signal to a waiting thread, which will attempt to grab the lock.  In this
  // case, the waiting thread has first dibs on the lock, and any new threads
  // will still treat it as locked and wait until there are no more waiters.

  // First try to acquire the lock without suspending the thread.  This only
  // works if the waiter count is 0; this way, we give priority to threads
  // that are already waiting for the event.
  if (InterlockedCompareExchangePointer(lock, (void *)lock_bit, nullptr) == nullptr) {
    // Got the lock on the first try.
    return;
  }

  // On multi-core systems, we keep trying for the configured spin_count.
  const unsigned int max_spins = effective_spin_count;
  for (unsigned int spins = 0; spins < max_spins; ++spins) {
    if (InterlockedCompareExchangePointer(lock, (void *)lock_bit, nullptr) == nullptr) {
      // We managed to acquire the lock.
      return;
    }

    // Emit the pause instruction.  This is NOT a thread yield.
    YieldProcessor();
  }

  // Looks like we might have to go to sleep for a while using an event.
  HANDLE event = lock[1];
  if (event == nullptr) {
    // We don't have an event yet.  Create an auto-reset event.
    HANDLE new_event = CreateEvent(nullptr, false, false, nullptr);
    while (new_event == nullptr) {
      // Hmm, out of memory?  Just yield to another thread for now until the
      // lock is either freed or until we can create an event.
      Sleep(1);
      if (InterlockedCompareExchangePointer(lock, (void *)lock_bit, nullptr) == 0) {
        return;
      }
      new_event = CreateEvent(nullptr, false, false, nullptr);
    }

    // Push the new event.
    event = InterlockedCompareExchangePointer(lock + 1, new_event, nullptr);
    if (event == nullptr) {
      // Set successfully.
      event = new_event;
    } else {
      // Another thread created an event; delete ours and use that one instead.
      CloseHandle(new_event);
    }
  }

  // OK, now we have an event.  We need to let the unlock() function know that
  // we are waiting.
  while (true) {
    uintptr_t waiters = (uintptr_t)lock[0];
    if (waiters == 0) {
      // It became unlocked while we were creating an event.  Quick, grab it.
      if (InterlockedCompareExchangePointer(lock, (void *)lock_bit, nullptr) == nullptr) {
        return;
      }
    }

    // If the lock bit gets unset while we try this, just keep trying.  It
    // would be dangerous to increment this while the lock bit is unset.
    waiters |= lock_bit;
    uintptr_t new_waiters = (uintptr_t)InterlockedCompareExchangePointer(lock, (void *)(waiters + 1), (void *)waiters);
    if (new_waiters == waiters) {
      // Made the change successfully.
      break;
    } else if (new_waiters == 0) {
      // It just became unlocked.  Quick, grab it.
      if (InterlockedCompareExchangePointer(lock, (void *)lock_bit, nullptr) == nullptr) {
        return;
      }
    }
    YieldProcessor();
  }

  // Sleep well, thread.
  while (true) {
    WaitForSingleObjectEx(event, INFINITE, FALSE);

    // We were woken up.  Does that mean the lock can be ours?
    while (true) {
      uintptr_t waiters = (uintptr_t)lock[0];
      if (waiters & lock_bit) {
        // False alarm.  Go back to sleep.
        break;
      }
      assert(waiters > 0);

      // Grab the lock immediately, and simultaneously tell it that we are no
      // longer waiting.
      uintptr_t new_waiters = (uintptr_t)InterlockedCompareExchangePointer(lock, (void *)((waiters - 1) | lock_bit), (void *)waiters);
      if (new_waiters == waiters) {
        // The lock is ours.
        return;
      } else if (new_waiters & lock_bit) {
        // Another thread beat us to it.  Go back to sleep.
        break;
      }
      YieldProcessor();
    }
  }

  // Never supposed to get here.
  assert(false);
}

/**
 * Windows XP implementation of try_lock().
 */
static BOOL __stdcall try_lock_xp(volatile PVOID *lock) {
  return (InterlockedCompareExchangePointer(lock, (void *)lock_bit, nullptr) == nullptr);
}

/**
 * Windows XP implementation of unlock().
 */
static void __stdcall unlock_xp(volatile PVOID *lock) {
  // Clear the lock flag.
#ifdef _WIN64
  uintptr_t waiters = _InterlockedAnd64((volatile __int64 *)lock, ~lock_bit);
#else
  uintptr_t waiters = _InterlockedAnd((volatile long *)lock, ~lock_bit);
#endif

  // If this triggers, the lock wasn't held to begin with.
  assert((waiters & lock_bit) != 0);

  // Have any threads begun to sleep (or are about to) waiting for this lock?
  if ((waiters & ~lock_bit) == 0) {
    // No contention, nothing to do.
    return;
  } else {
    // By signalling the auto-resetting event, we wake up one waiting thread.
    HANDLE event = lock[1];
    assert(event != nullptr);
    SetEvent(event);
  }
}

/**
 * Windows XP implementation to wait for a condition variable.
 */
static BOOL __stdcall
cvar_wait_xp(volatile PVOID *cvar, volatile PVOID *lock, DWORD timeout, ULONG) {
  // Increment the number of waiters.
#ifdef _WIN64
  _InterlockedIncrement64((volatile __int64 *)cvar);
#else
  _InterlockedIncrement((volatile long *)cvar);
#endif

  // Make sure we have two events created: one auto-reset event and one
  // manual-reset, to handle signal and broadcast, respectively.
  if (cvar[1] == nullptr) {
    cvar[1] = CreateEvent(nullptr, false, false, nullptr);
  }
  if (cvar[2] == nullptr) {
    cvar[2] = CreateEvent(nullptr, true, false, nullptr);
  }

  // It's ok to release the external_mutex here since Win32 manual-reset
  // events maintain state when used with SetEvent(). This avoids the "lost
  // wakeup" bug...
  unlock_xp(lock);

  // Wait for either event to become signaled due to notify() being called or
  // notify_all() being called.
  int result = WaitForMultipleObjects(2, (const HANDLE *)(cvar + 1), FALSE, timeout);

  // Decrement the counter.  If it reached zero, we were the last waiter.
#ifdef _WIN64
  bool nonzero = (_InterlockedDecrement64((volatile __int64 *)cvar) != 0);
#else
  bool nonzero = (_InterlockedDecrement((volatile long *)cvar) != 0);
#endif
  bool last_waiter = (result == WAIT_OBJECT_0 + 1 && !nonzero);

  // Some thread called notify_all().
  if (last_waiter) {
    // We're the last waiter to be notified or to stop waiting, so reset the
    // manual event.
    ResetEvent(cvar[2]);
  }

  // Reacquire the <external_mutex>.
  lock_xp(lock);
  return TRUE;
}

/**
 * Wakes one thread waiting for a condition variable.
 */
static void __stdcall
cvar_notify_one_xp(volatile PVOID *cvar) {
  // If there are any waiters, signal one of them to wake up by signalling the
  // auto-reset event.
  if ((uintptr_t)cvar[0] > 0) {
    SetEvent(cvar[1]);
  }
}

/**
 * Wakes all threads waiting for a condition variable.
 */
static void __stdcall
cvar_notify_all_xp(volatile PVOID *cvar) {
  // If there are any waiters, signal the manual-reset event, which will be
  // reset by the last thread to wake up.
  if ((uintptr_t)cvar[0] > 0) {
    SetEvent(cvar[2]);
  }
}

#endif  // _WIN32_WINNT < 0x0600

/**
 * This is put initially in the _lock slot; it makes sure that the lock
 * functions get initialized the first time someone tries to grab a lock.
 */
void __stdcall MutexWin32Impl::
lock_initially(volatile PVOID *lock) {
  MutexWin32Impl::init_lock_funcs();
  MutexWin32Impl::_funcs._lock(lock);
}

/**
 * This is put initially in the _try_lock slot; it makes sure that the lock
 * functions get initialized the first time someone tries to grab a lock.
 */
BOOL __stdcall MutexWin32Impl::
try_lock_initially(volatile PVOID *lock) {
  MutexWin32Impl::init_lock_funcs();
  return MutexWin32Impl::_funcs._try_lock(lock);
}

/**
 * This gets put initially in the _unlock slot and should never be called,
 * since the initial lock/try_lock implementation will replace the pointers.
 */
void __stdcall MutexWin32Impl::
unlock_initially(volatile PVOID *) {
#if !defined(NDEBUG) || defined(DEBUG_THREADS)
  std::cerr << "Attempt to release a mutex at static init time before acquiring it!\n";
  assert(false);
#endif
}

/**
 * Same as above for condition variables.
 */
static BOOL __stdcall
cvar_wait_initially(volatile PVOID *cvar, volatile PVOID *lock, DWORD timeout, ULONG) {
#if !defined(NDEBUG) || defined(DEBUG_THREADS)
  std::cerr << "Attempt to wait for condition variable at static init time before acquiring mutex!\n";
  assert(false);
#endif
  return FALSE;
}

/**
 * Does nothing.
 */
static void __stdcall
noop(volatile PVOID *) {
}

// We initially set the function pointers to functions that do initialization
// first.
MutexWin32Impl::LockFunctions MutexWin32Impl::_funcs = {
  &MutexWin32Impl::lock_initially,
  &MutexWin32Impl::try_lock_initially,
#ifndef NDEBUG
  &MutexWin32Impl::unlock_initially,
#else
  &noop,
#endif

  &cvar_wait_initially,
#ifndef NDEBUG
  &MutexWin32Impl::unlock_initially,
  &MutexWin32Impl::unlock_initially,
#else
  &noop,
  &noop,
#endif
};

/**
 * Called the first time a lock is grabbed.
 */
void MutexWin32Impl::
init_lock_funcs() {
  if (MutexWin32Impl::_funcs._lock != &MutexWin32Impl::lock_initially) {
    // Already initialized.
    return;
  }

#if _WIN32_WINNT >= 0x0600
  _funcs._lock = (LockFunc)AcquireSRWLockExclusive;
  _funcs._try_lock = (TryLockFunc)TryAcquireSRWLockExclusive;
  _funcs._unlock = (LockFunc)ReleaseSRWLockExclusive;
  _funcs._cvar_wait = (CondWaitFunc)SleepConditionVariableSRW;
  _funcs._cvar_notify_one = (LockFunc)WakeConditionVariable;
  _funcs._cvar_notify_all = (LockFunc)WakeAllConditionVariable;
#else
  // We don't need to be very thread safe here.  This can only ever be called
  // at static init time, when there is still only one thread.
  if (prefer_srwlock) {
    HMODULE module = GetModuleHandleA("kernel32");
    if (module != nullptr) {
      _funcs._lock = (LockFunc)GetProcAddress(module, "AcquireSRWLockExclusive");
      _funcs._try_lock = (TryLockFunc)GetProcAddress(module, "TryAcquireSRWLockExclusive");
      _funcs._unlock = (LockFunc)GetProcAddress(module, "ReleaseSRWLockExclusive");
      _funcs._cvar_wait = (CondWaitFunc)GetProcAddress(module, "SleepConditionVariableSRW");
      _funcs._cvar_notify_one = (LockFunc)GetProcAddress(module, "WakeConditionVariable");
      _funcs._cvar_notify_all = (LockFunc)GetProcAddress(module, "WakeAllConditionVariable");
      if (_funcs._lock != nullptr &&
          _funcs._try_lock != nullptr &&
          _funcs._unlock != nullptr &&
          _funcs._cvar_wait != nullptr &&
          _funcs._cvar_notify_one != nullptr &&
          _funcs._cvar_notify_all != nullptr) {
        return;
      }
    }
  }

  // Fall back to our custom Event-based implementation on Windows XP.
  _funcs._lock = &lock_xp;
  _funcs._try_lock = &try_lock_xp;
  _funcs._unlock = &unlock_xp;
  _funcs._cvar_wait = &cvar_wait_xp;
  _funcs._cvar_notify_one = &cvar_notify_one_xp;
  _funcs._cvar_notify_all = &cvar_notify_all_xp;

  // Are we on a multi-core system?  If so, enable the spinlock.
  SYSTEM_INFO sysinfo;
  GetSystemInfo(&sysinfo);
  if (sysinfo.dwNumberOfProcessors > 1) {
    effective_spin_count = spin_count;
  } else {
    effective_spin_count = 0;
  }
#endif  // _WIN32_WINNT < 0x0600
}

/**
 * Ensures that the lock functions are initialized at static init time.  This
 * prevents us from having to implement synchronization in our initialization.
 */
class LockFuncsInitializer {
public:
  LockFuncsInitializer() {
    MutexWin32Impl::init_lock_funcs();
  }
};

static LockFuncsInitializer _lock_funcs_init;

/**
 *
 */
ReMutexWin32Impl::
ReMutexWin32Impl() {
  InitializeCriticalSectionAndSpinCount(&_lock, spin_count);
}

#endif  // WIN32_VC
