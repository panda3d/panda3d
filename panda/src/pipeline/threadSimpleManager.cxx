// Filename: threadSimpleManager.cxx
// Created by:  drose (19Jun07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "threadSimpleManager.h"
#include "threadSimpleImpl.h"
#include "blockerSimple.h"

bool ThreadSimpleManager::_pointers_initialized;
ThreadSimpleManager *ThreadSimpleManager::_global_ptr;

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleManager::Constructor
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
ThreadSimpleManager::
ThreadSimpleManager() {
  _clock = TrueClock::get_global_ptr();
  _waiting_for_exit = NULL;
  nassertv(_current_thread == NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleManager::enqueue_ready
//       Access: Public
//  Description: Adds the indicated thread to the ready queue.  The
//               thread will be executed when its turn comes.  If the
//               thread is not the currently executing thread, its
//               _context should be filled appropriately.
////////////////////////////////////////////////////////////////////
void ThreadSimpleManager::
enqueue_ready(ThreadSimpleImpl *thread) {
  _ready.push_back(thread);
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleManager::enqueue_sleep
//       Access: Public
//  Description: Adds the indicated thread to the sleep queue, until
//               the indicated number of seconds have elapsed.  Then
//               the thread will be automatically moved to the ready
//               queue.
////////////////////////////////////////////////////////////////////
void ThreadSimpleManager::
enqueue_sleep(ThreadSimpleImpl *thread, double seconds) {
  double now = get_current_time();
  thread->_start_time = now + seconds;
  _sleeping.push_back(thread);
  push_heap(_sleeping.begin(), _sleeping.end(), CompareStartTime());
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleManager::enqueue_block
//       Access: Public
//  Description: Adds the indicated thread to the blocked queue for
//               the indicated blocker.  The thread will be awoken by
//               a later call to unblock_one() or unblock_all().
////////////////////////////////////////////////////////////////////
void ThreadSimpleManager::
enqueue_block(ThreadSimpleImpl *thread, BlockerSimple *blocker) {
  _blocked[blocker].push_back(thread);
  blocker->_flags |= BlockerSimple::F_has_waiters;
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleManager::unblock_one
//       Access: Public
//  Description: Unblocks one thread waiting on the indicated blocker,
//               if any.
////////////////////////////////////////////////////////////////////
void ThreadSimpleManager::
unblock_one(BlockerSimple *blocker) {
  Blocked::iterator bi = _blocked.find(blocker);
  if (bi != _blocked.end()) {
    nassertv(blocker->_flags & BlockerSimple::F_has_waiters);

    FifoThreads &threads = (*bi).second;
    nassertv(!threads.empty());
    ThreadSimpleImpl *thread = threads.front();
    threads.pop_front();
    _ready.push_back(thread);
    if (threads.empty()) {
      blocker->_flags &= ~BlockerSimple::F_has_waiters;
      _blocked.erase(bi);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleManager::unblock_all
//       Access: Public
//  Description: Unblocks all threads waiting on the indicated
//               blocker.
////////////////////////////////////////////////////////////////////
void ThreadSimpleManager::
unblock_all(BlockerSimple *blocker) {
  Blocked::iterator bi = _blocked.find(blocker);
  if (bi != _blocked.end()) {
    nassertv(blocker->_flags & BlockerSimple::F_has_waiters);

    FifoThreads &threads = (*bi).second;
    nassertv(!threads.empty());
    while (!threads.empty()) {
      ThreadSimpleImpl *thread = threads.front();
      threads.pop_front();
      _ready.push_back(thread);
    }
    blocker->_flags &= ~BlockerSimple::F_has_waiters;
    _blocked.erase(bi);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleManager::enqueue_finished
//       Access: Public
//  Description: Adds the indicated thread to the finished queue.  
//               The manager will drop the reference count on the
//               indicated thread at the next epoch.  (A thread can't
//               drop its own reference count while it is running,
//               since that might deallocate its own stack.)
////////////////////////////////////////////////////////////////////
void ThreadSimpleManager::
enqueue_finished(ThreadSimpleImpl *thread) {
  _finished.push_back(thread);
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleManager::preempt
//       Access: Public
//  Description: Moves the indicated thread to the head of the ready
//               queue.  If it is not already on the ready queue, does
//               nothing.
////////////////////////////////////////////////////////////////////
void ThreadSimpleManager::
preempt(ThreadSimpleImpl *thread) {
  FifoThreads::iterator ti;
  ti = find(_ready.begin(), _ready.end(), thread);
  if (ti != _ready.end()) {
    _ready.erase(ti);
    _ready.push_front(thread);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleManager::next_context
//       Access: Public
//  Description: Switches out the currently executing thread and
//               chooses a new thread for execution.  Before calling
//               this, the current thread should have already
//               re-enqueued itself with a call to enqueue(), if it
//               intends to run again.
//
//               This will fill in the current thread's _context
//               member appropriately, and then change the global
//               current_thread pointer.
////////////////////////////////////////////////////////////////////
void ThreadSimpleManager::
next_context() {
  // Mark the current thread's resume point.

#ifdef HAVE_PYTHON
  // Query the current Python thread state.
  _current_thread->_python_state = PyThreadState_Swap(NULL);
  PyThreadState_Swap(_current_thread->_python_state);
#endif  // HAVE_PYTHON

  if (setjmp(_current_thread->_context) != 0) {
    // We have returned from a longjmp, and are now resuming the
    // current thread.
#ifdef HAVE_PYTHON
    PyThreadState_Swap(_current_thread->_python_state);
#endif  // HAVE_PYTHON

    return;
  }

  while (!_finished.empty() && _finished.front() != _current_thread) {
    ThreadSimpleImpl *finished_thread = _finished.front();
    _finished.pop_front();
    unref_delete(finished_thread->_parent_obj);
  }

  _current_thread = NULL;

  double now = get_current_time();

  if (!_sleeping.empty()) {
    wake_sleepers(now);
  }

  // Choose a new thread to execute.
  while (_ready.empty()) {
    // No threads are ready.  They must all be sleeping.
    if (_sleeping.empty()) {
      // No threads at all!
      if (!_blocked.empty()) {
        thread_cat.error()
          << "Deadlock!  All threads blocked.\n";
        report_deadlock();
        abort();
      }

      // All threads have finished execution.
      if (_waiting_for_exit != NULL) {
        // And one thread--presumably the main thread--was waiting for
        // that.
        _ready.push_back(_waiting_for_exit);
        _waiting_for_exit = NULL;
        break;
      }

      // No threads are queued anywhere.  This is kind of an error,
      // since normally the main thread, at least, should be queued
      // somewhere.
      thread_cat.error()
        << "All threads disappeared!\n";
      exit(0);
    }

    double wait = _sleeping.front()->_start_time - now;
    if (wait > 0.0) {
      if (thread_cat.is_spam()) {
        thread_cat.spam()
          << "Sleeping " << wait << " seconds\n";
      }
      system_sleep(wait);
    }
    now = get_current_time();
    wake_sleepers(now);
  }

  _current_thread = _ready.front();
  _ready.pop_front();
  _current_thread->_start_time = now;

  // All right, the thread is ready to roll.  Begin.
  if (thread_cat.is_spam()) {
    thread_cat.spam()
      << "Switching to " << *_current_thread->_parent_obj << "\n";
  }
  longjmp(_current_thread->_context, 1);

  // Shouldn't get here.
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleManager::prepare_for_exit
//       Access: Public
//  Description: Blocks until all running threads (other than the
//               current thread) have finished.  You should probably
//               only call this from the main thread.
////////////////////////////////////////////////////////////////////
void ThreadSimpleManager::
prepare_for_exit() {
  nassertv(_waiting_for_exit == NULL);
  _waiting_for_exit = _current_thread;

  // At this point, any non-joinable threads on any of the queues are
  // automatically killed.
  kill_non_joinable(_ready);

  Blocked::iterator bi = _blocked.begin();
  while (bi != _blocked.end()) {
    Blocked::iterator bnext = bi;
    ++bnext;
    BlockerSimple *blocker = (*bi).first;
    FifoThreads &threads = (*bi).second;
    kill_non_joinable(threads);
    if (threads.empty()) {
      blocker->_flags &= ~BlockerSimple::F_has_waiters;
      _blocked.erase(bi);
    }
    bi = bnext;
  }

  kill_non_joinable(_sleeping);

  next_context();

  while (!_finished.empty() && _finished.front() != _current_thread) {
    ThreadSimpleImpl *finished_thread = _finished.front();
    _finished.pop_front();
    unref_delete(finished_thread->_parent_obj);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleManager::set_current_thread
//       Access: Public
//  Description: Sets the initial value of the current_thread pointer,
//               i.e. the main thread.  It is valid to call this
//               method only exactly once.
////////////////////////////////////////////////////////////////////
void ThreadSimpleManager::
set_current_thread(ThreadSimpleImpl *current_thread) {
  nassertv(_current_thread == (ThreadSimpleImpl *)NULL);
  _current_thread = current_thread;
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleManager::init_pointers
//       Access: Private, Static
//  Description: Should be called at startup to initialize the
//               simple threading system.
////////////////////////////////////////////////////////////////////
void ThreadSimpleManager::
init_pointers() {
  if (!_pointers_initialized) {
    _pointers_initialized = true;
    _global_ptr = new ThreadSimpleManager;
    Thread::get_main_thread();

#ifdef HAVE_PYTHON
    // Ensure that the Python threading system is initialized and ready
    // to go.
    PyEval_InitThreads();
#endif
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleManager::wake_sleepers
//       Access: Private
//  Description: Moves any threads due to wake up from the sleeping
//               queue to the ready queue.
////////////////////////////////////////////////////////////////////
void ThreadSimpleManager::
wake_sleepers(double now) {
  while (!_sleeping.empty() && _sleeping.front()->_start_time <= now) {
    ThreadSimpleImpl *thread = _sleeping.front();
    pop_heap(_sleeping.begin(), _sleeping.end(), CompareStartTime());
    _sleeping.pop_back();
    _ready.push_back(thread);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleManager::system_sleep
//       Access: Private, Static
//  Description: Calls the appropriate system sleep function to sleep
//               the whole process for the indicated number of
//               seconds.
////////////////////////////////////////////////////////////////////
void ThreadSimpleManager::
system_sleep(double seconds) {
#ifdef WIN32
  Sleep((int)(seconds * 1000));

#else
  struct timespec rqtp;
  rqtp.tv_sec = time_t(seconds);
  rqtp.tv_nsec = long((seconds - (double)rqtp.tv_sec) * 1000000000.0);
  nanosleep(&rqtp, NULL);
#endif  // WIN32
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleManager::report_deadlock
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void ThreadSimpleManager::
report_deadlock() {
  Blocked::iterator bi;
  for (bi = _blocked.begin(); bi != _blocked.end(); ++bi) {
    BlockerSimple *blocker = (*bi).first;
    FifoThreads &threads = (*bi).second;
    thread_cat.info()
      << "On blocker " << blocker << ":\n";
    FifoThreads::iterator ti;
    for (ti = threads.begin(); ti != threads.end(); ++ti) {
      ThreadSimpleImpl *thread = (*ti);
      thread_cat.info()
        << "  " << *thread->_parent_obj << "\n";
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleManager::kill_non_joinable
//       Access: Private
//  Description: Removes any non-joinable threads from the indicated
//               queue and marks them killed.
////////////////////////////////////////////////////////////////////
void ThreadSimpleManager::
kill_non_joinable(ThreadSimpleManager::FifoThreads &threads) {
  FifoThreads new_threads;
  FifoThreads::iterator ti;
  for (ti = threads.begin(); ti != threads.end(); ++ti) {
    ThreadSimpleImpl *thread = (*ti);
    if (thread->_joinable) {
      new_threads.push_back(thread);
    } else {
      if (thread_cat.is_debug()) {
        thread_cat.debug()
          << "Killing " << *thread->_parent_obj << "\n";
      }
      thread->_status = ThreadSimpleImpl::S_killed;
      enqueue_finished(thread);
    }
  }

  threads.swap(new_threads);
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleManager::kill_non_joinable
//       Access: Private
//  Description: Removes any non-joinable threads from the indicated
//               queue and marks them killed.
////////////////////////////////////////////////////////////////////
void ThreadSimpleManager::
kill_non_joinable(ThreadSimpleManager::Sleeping &threads) {
  Sleeping new_threads;
  Sleeping::iterator ti;
  for (ti = threads.begin(); ti != threads.end(); ++ti) {
    ThreadSimpleImpl *thread = (*ti);
    if (thread->_joinable) {
      new_threads.push_back(thread);
    } else {
      if (thread_cat.is_debug()) {
        thread_cat.debug()
          << "Killing " << *thread->_parent_obj << "\n";
      }
      thread->_status = ThreadSimpleImpl::S_killed;
      enqueue_finished(thread);
    }
  }
  make_heap(new_threads.begin(), new_threads.end(), CompareStartTime());
  threads.swap(new_threads);
}
