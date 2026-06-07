/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file epochManager.h
 * @author Maxwell Dreytser
 * @date 2026-05-17
 *
 * Epoch-based reclamation for PipelineCycler-published CycleData.  Writers
 * copy-on-write and atomically swap the published pointer; readers load it
 * lock-free (no refcount, no shared write) and dereference it for the lifetime
 * of their critical section.  A replaced pointer is not freed immediately --
 * it is stamped with the global epoch and queued, then freed once every thread
 * in a critical section has been observed at a strictly later epoch.
 *
 * A critical section is delimited by Thread::epoch_enter/leave, called by the
 * CycleDataReader/Writer RAII types and, to amortize the cost across many
 * readers, by the framework once per task/frame (AsyncTaskManager::poll,
 * GraphicsEngine::render_frame, AsyncTaskChain task).
 */
#ifndef EPOCHMANAGER_H
#define EPOCHMANAGER_H

#include "pandabase.h"
#include "patomic.h"
#include "pvector.h"

#include <cstdint>

class Thread;
class CycleData;

#ifdef THREADED_PIPELINE

#ifndef CPPPARSER
// Per-OS-thread epoch state (lives on Thread; see Thread::epoch_participant).
// slot == 0 means quiescent, else the epoch observed at the outermost enter.
struct EpochParticipant {
  uint32_t depth = 0;
  patomic<uint64_t> slot{0};
  EpochParticipant *next = nullptr;
  uint32_t reclaim_ticks = 0;
  bool registered = false;
  bool online = false;
  ~EpochParticipant();
};
#else  // CPPPARSER
struct EpochParticipant {};
#endif  // CPPPARSER

class EXPCL_PANDA_PIPELINE EpochManager {
public:
  ALWAYS_INLINE static void enter(EpochParticipant &p);
  ALWAYS_INLINE static void leave(EpochParticipant &p);
  ALWAYS_INLINE static void consider_reclaim(EpochParticipant &p);

  static void register_participant(EpochParticipant *p);
  static void unregister_participant(EpochParticipant *p);

  static void go_online(EpochParticipant &p);

  static void retire(CycleData *cd);
  static void try_advance_epoch();
  static size_t try_reclaim(size_t budget = ~size_t(0));

  static void register_thread(Thread *t, int stage);
  static void unregister_thread(Thread *t, int stage);
  static void thread_stage_changed(int old_stage, int new_stage);

  static uint64_t get_global_epoch();
  static size_t get_retired_count();
  static int get_threads_at_stage(int stage);

  static bool is_reclaiming();

  ALWAYS_INLINE static bool try_begin_inplace_write(int stage);
  ALWAYS_INLINE static void end_inplace_write(int stage);
  ALWAYS_INLINE static bool inplace_allowed_hint(int stage);

  static constexpr int MAX_STAGES = 4;

#ifndef CPPPARSER
private:
  EpochManager() = delete;
  ~EpochManager() = delete;

  static void stage_occupancy_inc(int stage);
  static void stage_occupancy_dec(int stage);

  static patomic<uint64_t> _global_epoch;  // 0 reserved for "quiescent"

  // Participants currently inside a critical section.
  static patomic<int> _active_cs_count;
  // Approximate retired-list size, so leave() can skip the lock when nothing
  // is pending.
  static patomic<size_t> _retired_count;

  // Retired-entry count at which an online thread drains on its way back to
  // depth 0.
  static constexpr size_t checkpoint_threshold = 256;

  // Live occupant count per stage; the in-place fast path gates on count <= 1.
  static patomic<int> _threads_at_stage[MAX_STAGES];
  static patomic<int> _inplace_writers_at_stage[MAX_STAGES];

#endif  // CPPPARSER
};

#ifndef CPPPARSER
#include "epochManager.I"
#endif  // CPPPARSER

#else  // !THREADED_PIPELINE

class EXPCL_PANDA_PIPELINE EpochManager {
public:
  ALWAYS_INLINE static void enter() {}
  ALWAYS_INLINE static void leave() {}
  ALWAYS_INLINE static void consider_reclaim() {}
  ALWAYS_INLINE static void retire(CycleData *) {}
  ALWAYS_INLINE static void try_advance_epoch() {}
  ALWAYS_INLINE static size_t try_reclaim(size_t = 0) { return 0; }
  ALWAYS_INLINE static void register_thread(Thread *, int) {}
  ALWAYS_INLINE static void unregister_thread(Thread *, int) {}
  ALWAYS_INLINE static void thread_stage_changed(int, int) {}
  ALWAYS_INLINE static uint64_t get_global_epoch() { return 0; }
  ALWAYS_INLINE static size_t get_retired_count() { return 0; }
  ALWAYS_INLINE static int get_threads_at_stage(int) { return 0; }
  ALWAYS_INLINE static bool try_begin_inplace_write(int) { return false; }
  ALWAYS_INLINE static void end_inplace_write(int) {}
  ALWAYS_INLINE static bool inplace_allowed_hint(int) { return false; }
};

#endif  // THREADED_PIPELINE

#endif  // EPOCHMANAGER_H
