/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mutexDebug.cxx
 * @author drose
 * @date 2006-02-13
 */

#include "mutexDebug.h"
#include "thread.h"
#include "config_pipeline.h"

#ifdef DEBUG_THREADS

using std::ostream;
using std::ostringstream;

int MutexDebug::_pstats_count = 0;
MutexTrueImpl *MutexDebug::_global_lock;

/**
 *
 */
MutexDebug::
MutexDebug(const std::string &name, bool allow_recursion, bool lightweight) :
  Namable(name),
  _allow_recursion(allow_recursion),
  _lightweight(lightweight),
  _locking_thread(nullptr),
  _lock_count(0),
  _deleted_name(nullptr),
  _cvar_impl(*get_global_lock())
{
#ifndef SIMPLE_THREADS
  // If we're using real threads, there's no such thing as a lightweight
  // mutex.
  _lightweight = false;
#endif
}

/**
 *
 */
MutexDebug::
~MutexDebug() {
  nassertv(_locking_thread == nullptr && _lock_count == 0);

  // If the config variable says to, allocate (and leak) a string name for the
  // mutex, so we can report which mutex it is that has destructed after the
  // fact.
  if (name_deleted_mutexes) {
    ostringstream strm;
    strm << *this;
    std::string name = strm.str();
    _deleted_name = strdup((char *)name.c_str());
  }

  // Put a distinctive, bogus lock count in upon destruction, so we'll be more
  // likely to notice a floating pointer.
  _lock_count = -100;
}

/**
 * This method is declared virtual in MutexDebug, but non-virtual in
 * MutexDirect.
 */
void MutexDebug::
output(ostream &out) const {
  if (_lightweight) {
    out << "Light";
  }
  if (_allow_recursion) {
    out << "ReMutex " << get_name() << " " << (void *)this;
  } else {
    out << "Mutex " << get_name() << " " << (void *)this;
  }
}

/**
 * Reports the mutex as well as the thread that is currently holding it, if
 * any.
 */
void MutexDebug::
output_with_holder(ostream &out) const {
  _global_lock->lock();
  output(out);
  if (_locking_thread != nullptr) {
    out << " (held by " << *_locking_thread << ")\n";
  }
  _global_lock->unlock();
}

/**
 * Intended to be called only by PStatClientImpl::client_connect(), this tells
 * the global mutex system that PStats is active.  Once PStats is active, all
 * "light" mutexes are treated the same as full mutexes.
 */
void MutexDebug::
increment_pstats() {
  _global_lock->lock();
  ++_pstats_count;
  _global_lock->unlock();
}

/**
 * Intended to be called only by PStatClientImpl::client_disconnect(), this
 * tells the global mutex system that PStats is no longer active.
 */
void MutexDebug::
decrement_pstats() {
  _global_lock->lock();
  --_pstats_count;
  _global_lock->unlock();
}

/**
 * The private implementation of acquire() assumes that _lock_impl is held.
 */
void MutexDebug::
do_lock(Thread *current_thread) {
  // If this assertion is triggered, you tried to lock a recently-destructed
  // mutex.
  nassertd(_lock_count != -100) {
    pipeline_cat.error()
      << "Destructed mutex: " << (void *)this << "\n";
    if (name_deleted_mutexes && _deleted_name != nullptr) {
      pipeline_cat.error()
        << _deleted_name << "\n";
    } else {
      pipeline_cat.error()
        << "Configure name-deleted-mutexes 1 to see the mutex name.\n";
    }
    return;
  }

  if (_locking_thread == nullptr) {
    // The mutex is not already locked by anyone.  Lock it.
    _locking_thread = current_thread;
    ++_lock_count;
    nassertv(_lock_count == 1);

  } else if (_locking_thread == current_thread) {
    // The mutex is already locked by this thread.  Increment the lock count.
    nassertv(_lock_count > 0);
    if (!_allow_recursion) {
      ostringstream ostr;
      ostr << *current_thread << " attempted to double-lock non-reentrant "
           << *this;
      nassert_raise(ostr.str());
    }
    ++_lock_count;

  } else {
    // The mutex is locked by some other thread.

    if (_lightweight && _pstats_count == 0) {
      // In this case, it's not a real mutex.  Just watch it go by.
      MissedThreads::iterator mi = _missed_threads.insert(MissedThreads::value_type(current_thread, 0)).first;
      if ((*mi).second == 0) {
        ostringstream ostr;
        ostr << *current_thread << " not stopped by " << *this << " (held by "
             << *_locking_thread << ")\n";
        nassert_raise(ostr.str());
      } else {
        if (!_allow_recursion) {
          ostringstream ostr;
          ostr << *current_thread << " attempted to double-lock non-reentrant "
               << *this;
          nassert_raise(ostr.str());
        }
      }
      ++((*mi).second);

    } else {
      // This is the real case.  It's a real mutex, so block if necessary.

      // Check for deadlock.
      MutexDebug *next_mutex = this;
      while (next_mutex != nullptr) {
        if (next_mutex->_locking_thread == current_thread) {
          // Whoops, the thread is blocked on me!  Deadlock!
          report_deadlock(current_thread);
          nassert_raise("Deadlock");
          return;
        }
        Thread *next_thread = next_mutex->_locking_thread;
        if (next_thread == nullptr) {
          // Looks like this mutex isn't actually locked, which means the last
          // thread isn't really blocked--it just hasn't woken up yet to
          // discover that.  In any case, no deadlock.
          break;
        }

        // The last thread is blocked on this "next thread"'s mutex, but what
        // mutex is the next thread blocked on?
        next_mutex = next_thread->_blocked_on_mutex;
      }

      // OK, no deadlock detected.  Carry on.
      current_thread->_blocked_on_mutex = this;

      // Go to sleep on the condition variable until it's unlocked.

      if (thread_cat->is_debug()) {
        thread_cat.debug()
          << *current_thread << " blocking on " << *this << " (held by "
          << *_locking_thread << ")\n";
      }

      while (_locking_thread != nullptr) {
        thread_cat.debug()
          << *current_thread << " still blocking on " << *this << " (held by "
          << *_locking_thread << ")\n";
        _cvar_impl.wait();
      }

      if (thread_cat.is_debug()) {
        thread_cat.debug()
          << *current_thread << " acquired " << *this << "\n";
      }

      current_thread->_blocked_on_mutex = nullptr;

      _locking_thread = current_thread;
      ++_lock_count;
      nassertv(_lock_count == 1);
    }
  }
}

/**
 * The private implementation of acquire(false) assumes that _lock_impl is
 * held.
 */
bool MutexDebug::
do_try_lock(Thread *current_thread) {
  // If this assertion is triggered, you tried to lock a recently-destructed
  // mutex.
  nassertd(_lock_count != -100) {
    pipeline_cat.error()
      << "Destructed mutex: " << (void *)this << "\n";
    if (name_deleted_mutexes && _deleted_name != nullptr) {
      pipeline_cat.error()
        << _deleted_name << "\n";
    } else {
      pipeline_cat.error()
        << "Configure name-deleted-mutexes 1 to see the mutex name.\n";
    }
    return false;
  }

  bool acquired = true;
  if (_locking_thread == nullptr) {
    // The mutex is not already locked by anyone.  Lock it.
    _locking_thread = current_thread;
    ++_lock_count;
    nassertr(_lock_count == 1, false);

  } else if (_locking_thread == current_thread) {
    // The mutex is already locked by this thread.  Increment the lock count.
    nassertr(_lock_count > 0, false);
    if (!_allow_recursion) {
      // Non-recursive lock; return false.
      acquired = false;
    } else {
      ++_lock_count;
    }

  } else {
    // The mutex is locked by some other thread.  Return false.

    if (_lightweight && _pstats_count == 0) {
      // In this case, it's not a real mutex.  Just watch it go by.
      MissedThreads::iterator mi = _missed_threads.insert(MissedThreads::value_type(current_thread, 0)).first;
      if ((*mi).second == 0) {
        thread_cat.info()
          << *current_thread << " not stopped by " << *this << " (held by "
          << *_locking_thread << ")\n";
      } else {
        if (!_allow_recursion) {
          ostringstream ostr;
          ostr << *current_thread << " attempted to double-lock non-reentrant "
               << *this;
          nassert_raise(ostr.str());
        }
      }
      ++((*mi).second);

    } else {
      // This is the real case.
      acquired = false;
    }
  }

  return acquired;
}

/**
 * The private implementation of acquire() assumes that _lock_impl is held.
 */
void MutexDebug::
do_unlock() {
  // If this assertion is triggered, you tried to release a recently-
  // destructed mutex.
  nassertd(_lock_count != -100) {
    pipeline_cat.error()
      << "Destructed mutex: " << (void *)this << "\n";
    if (name_deleted_mutexes && _deleted_name != nullptr) {
      pipeline_cat.error()
        << _deleted_name << "\n";
    } else {
      pipeline_cat.error()
        << "Configure name-deleted-mutexes 1 to see the mutex name.\n";
    }
    return;
  }

  Thread *current_thread = Thread::get_current_thread();

  if (_locking_thread != current_thread) {
    // We're not holding this mutex.

    if (_lightweight) {
      // Not a real mutex.  This just means we blew past a mutex without
      // locking it, above.

      MissedThreads::iterator mi = _missed_threads.find(current_thread);
      nassertv(mi != _missed_threads.end());
      nassertv((*mi).second > 0);
      --((*mi).second);

      if ((*mi).second == 0) {
        _missed_threads.erase(mi);
      }

    } else {
      // In the real-mutex case, this is an error condition.
      ostringstream ostr;
      ostr << *current_thread << " attempted to release "
           << *this << " which it does not own";
      nassert_raise(ostr.str());
    }
    return;
  }

  nassertv(_lock_count > 0);

  --_lock_count;
  if (_lock_count == 0) {
    // That was the last lock held by this thread.  Release the lock.
    _locking_thread = nullptr;

    if (_lightweight) {
      if (!_missed_threads.empty()) {
        // Promote some other thread to be the honorary lock holder.
        MissedThreads::iterator mi = _missed_threads.begin();
        _locking_thread = (*mi).first;
        _lock_count = (*mi).second;
        _missed_threads.erase(mi);
        nassertv(_lock_count > 0);
      }
    } else {

      /*
      if (thread_cat.is_debug()) {
        thread_cat.debug()
          << *current_thread << " releasing " << *this << "\n";
      }
      */
      _cvar_impl.notify();
    }
  }
}

/**
 * The private implementation of debug_is_locked() assumes that _lock_impl is
 * held.
 */
bool MutexDebug::
do_debug_is_locked() const {
  Thread *current_thread = Thread::get_current_thread();
  if (_locking_thread == current_thread) {
    return true;
  }

  if (_lightweight) {
    MissedThreads::const_iterator mi = _missed_threads.find(current_thread);
    if (mi != _missed_threads.end()) {
      nassertr((*mi).second > 0, false);
      return true;
    }
  }

  return false;
}

/**
 * Reports a detected deadlock situation.  _lock_impl should be already held.
 */
void MutexDebug::
report_deadlock(Thread *current_thread) {
  thread_cat->error()
    << "\n\n"
    << "****************************************************************\n"
    << "*****                 Deadlock detected!                   *****\n"
    << "****************************************************************\n"
    << "\n";

  thread_cat.error()
    << *current_thread << " attempted to lock " << *this
    << " which is held by " << *_locking_thread << "\n";

  MutexDebug *next_mutex = this;
  Thread *next_thread = next_mutex->_locking_thread;
  next_mutex = next_thread->_blocked_on_mutex;
  while (next_mutex != nullptr) {
    thread_cat.error()
      << *next_thread << " is blocked waiting on "
      << *next_mutex << " which is held by "
      << *next_mutex->_locking_thread << "\n";
    next_thread = next_mutex->_locking_thread;
    next_mutex = next_thread->_blocked_on_mutex;
  }

  thread_cat.error()
    << "Deadlock!\n";
}

#endif  //  DEBUG_THREADS
