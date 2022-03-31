/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file threadSimpleManager.h
 * @author drose
 * @date 2007-06-18
 */

#ifndef THREADSIMPLEMANAGER_H
#define THREADSIMPLEMANAGER_H

#include "pandabase.h"
#include "selectThreadImpl.h"

#ifdef THREAD_SIMPLE_IMPL

#include "pdeque.h"
#include "pmap.h"
#include "pvector.h"
#include "trueClock.h"
#include "configVariableDouble.h"
#include <algorithm>

#ifdef HAVE_POSIX_THREADS
#include <pthread.h>  // for pthread_t, below
#endif
#ifdef WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>  // for DWORD, below
#endif

class Thread;
class ThreadSimpleImpl;
class BlockerSimple;
struct ThreadContext;

/**
 * This is the global object that selects the currently-active thread of the
 * various ThreadSimpleImpl objects running, when the currently-active thread
 * yields.
 *
 * This class only exists when we are using the ThreadSimple implementation,
 * which is to say, we are not using "real" threads.
 *
 * Generally, you shouldn't be calling these methods directly.  Call the
 * interfaces on Thread instead.
 */
class EXPCL_PANDA_PIPELINE ThreadSimpleManager {
private:
  ThreadSimpleManager();

public:
  void enqueue_ready(ThreadSimpleImpl *thread, bool volunteer);
  void enqueue_sleep(ThreadSimpleImpl *thread, double seconds);
  void enqueue_block(ThreadSimpleImpl *thread, BlockerSimple *blocker);
  bool unblock_one(BlockerSimple *blocker);
  bool unblock_all(BlockerSimple *blocker);
  void enqueue_finished(ThreadSimpleImpl *thread);
  void preempt(ThreadSimpleImpl *thread);
  void next_context();

  void prepare_for_exit();

  INLINE ThreadSimpleImpl *get_current_thread();
  void set_current_thread(ThreadSimpleImpl *current_thread);
  INLINE bool is_same_system_thread() const;
  void remove_thread(ThreadSimpleImpl *thread);
  static void system_sleep(double seconds);
  static void system_yield();

  double get_current_time() const;
  INLINE static ThreadSimpleManager *get_global_ptr();

  void write_status(std::ostream &out) const;

private:
  static void init_pointers();

  typedef pdeque<ThreadSimpleImpl *> FifoThreads;
  typedef pvector<ThreadSimpleImpl *> Sleeping;

  static void st_choose_next_context(struct ThreadContext *from_context, void *data);
  void choose_next_context(struct ThreadContext *from_context);
  void do_timeslice_accounting(ThreadSimpleImpl *thread, double now);
  void wake_sleepers(Sleeping &sleepers, double now);
  void wake_all_sleepers(Sleeping &sleepers);
  void report_deadlock();
  double determine_timeslice(ThreadSimpleImpl *chosen_thread);
  void kill_non_joinable(FifoThreads &threads);
  void kill_non_joinable(Sleeping &threads);

  // STL function object to sort the priority queue of sleeping threads.
  class CompareStartTime {
  public:
    INLINE bool operator ()(ThreadSimpleImpl *a, ThreadSimpleImpl *b) const;
  };

public:
  // Defined within the class to avoid static-init ordering problems.
  ConfigVariableDouble _simple_thread_epoch_timeslice;
  ConfigVariableDouble _simple_thread_volunteer_delay;
  ConfigVariableDouble _simple_thread_yield_sleep;
  ConfigVariableDouble _simple_thread_window;
  ConfigVariableDouble _simple_thread_low_weight;
  ConfigVariableDouble _simple_thread_normal_weight;
  ConfigVariableDouble _simple_thread_high_weight;
  ConfigVariableDouble _simple_thread_urgent_weight;

private:
  ThreadSimpleImpl *volatile _current_thread;

  // The list of ready threads: threads that are ready to execute right now.
  FifoThreads _ready;

  // The list of threads that are ready, but will not be executed until next
  // epoch (for instance, because they exceeded their timeslice budget this
  // epoch).
  FifoThreads _next_ready;

  // The list of threads that are blocked on some ConditionVar or Mutex.
  typedef pmap<BlockerSimple *, FifoThreads> Blocked;
  Blocked _blocked;

  // Priority queue (partially-ordered heap) of sleeping threads, based on
  // wakeup time.
  Sleeping _sleeping;

  // Priority queue (partially-ordered heap) of volunteer threads, based on
  // wakeup time.  This are threads that have voluntarily yielded a timeslice.
  // They are treated the same as sleeping threads, unless all threads are
  // sleeping.
  Sleeping _volunteers;

  // Threads which have finished execution and are awaiting cleanup.
  FifoThreads _finished;

  ThreadSimpleImpl *_waiting_for_exit;

  TrueClock *_clock;

  double _tick_scale;

  class TickRecord {
  public:
    unsigned int _tick_count;
    ThreadSimpleImpl *_thread;
  };
  typedef pdeque<TickRecord> TickRecords;
  TickRecords _tick_records;
  unsigned int _total_ticks;

  static bool _pointers_initialized;
  static ThreadSimpleManager *_global_ptr;
};

// We include this down here to avoid the circularity problem.
/* okcircular */
#include "threadSimpleImpl.h"

#include "threadSimpleManager.I"

#endif // THREAD_SIMPLE_IMPL

#endif
