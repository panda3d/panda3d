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
// Per-OS-thread epoch state (see Thread::epoch_participant for where it lives).
// slot == 0 means quiescent; otherwise it holds the epoch observed on the
// outermost enter.
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
// Interrogate names this type at the call sites in thread.h/.I but generates no
// bindings for it, and its parser trips on the real definition's patomic
// members; keep it opaque.
struct EpochParticipant {};
#endif  // CPPPARSER

class EXPCL_PANDA_PIPELINE EpochManager {
public:
  // Enter/leave a critical section.  The participant is passed in (rather than
  // looked up) to keep the hot path off the slow shared-library thread_local
  // access for bound threads; see Thread::epoch_participant.  Depth-counted --
  // only the outermost transition touches the atomic slot.
  ALWAYS_INLINE static void enter(EpochParticipant &p);
  ALWAYS_INLINE static void leave(EpochParticipant &p);

  // Opportunistic reclamation hook for Thread's yield/sleep family: lets
  // threads that never render or poll still make reclamation progress.  No-op
  // unless quiescent, and throttled.  Must use the same participant as
  // enter/leave on this thread, or the in-CS check would be wrong.
  ALWAYS_INLINE static void consider_reclaim(EpochParticipant &p);

  static void register_participant(EpochParticipant *p);
  static void unregister_participant(EpochParticipant *p);

  static void go_online(EpochParticipant &p);

  // The shared ExternalThread's participant: a function-local thread_local
  // reached through this exported accessor, because MSVC forbids a dll
  // interface on a thread_local variable.  Bound threads use a Thread member.
  static EpochParticipant &external_participant();

  // Stamp `cd` with the current epoch and queue it for reclamation; takes
  // ownership.  Callable from any thread.
  static void retire(CycleData *cd);

  // Bump the global epoch if every in-CS thread has observed the current one.
  static void try_advance_epoch();

  // Free retired entries older than the minimum observed epoch.  `budget`
  // caps per-call work.
  static size_t try_reclaim(size_t budget = ~size_t(0));

  // Stage-occupancy maintenance for the in-place fast path.  Driven by
  // Thread::acquire/release_stage_occupancy (when a thread begins/stops running
  // on its own stack) and set_pipeline_stage.  Independent of the epoch
  // participant registry, which is keyed to the OS thread.
  static void register_thread(Thread *t, int stage);
  static void unregister_thread(Thread *t, int stage);
  static void thread_stage_changed(int old_stage, int new_stage);

  static uint64_t get_global_epoch();
  static size_t get_retired_count();

  static bool is_reclaiming();

  // Per-stage in-place fast path: a write at stage S may mutate the published
  // CData in place (skipping make_copy) when S has a single occupying thread
  // (here) and the cycler's stage-S CData is unshared with other stages
  // (checked by the cycler).  Interlock against a thread arriving at S
  // mid-write: try_begin_inplace_write claims a slot and re-checks the count;
  // stage_occupancy_inc, on raising a stage to 2+, spin-drains in-flight
  // in-place writers first.  The count is a live gate, not sticky.
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

  // Number of participants currently inside a critical section.  When this
  // falls to 0 the pipeline is fully quiescent, which is the moment leave()
  // drains the retire queue so unframed / non-render contexts (unit tests,
  // synchronous loaders) reclaim promptly instead of never.
  static patomic<int> _active_cs_count;
  // Cheap hint of the retired-list size so the quiescent-drain in leave() can
  // skip the lock when there is nothing to reclaim (e.g. a pure read loop).
  static patomic<size_t> _retired_count;

  // Retired-entry count at which an online thread drains on its way back to
  // depth 0. This bounds an "online" thread's unreclaimed backlog while keeping
  // reclamation off the per-op path: an active frame retires well past this and
  // so drains about once per frame, while a tight unframed write loop drains
  // about once per this many writes instead of on every write.
  static constexpr size_t checkpoint_threshold = 256;

  // Live occupant count per stage; the fast path gates on count <= 1.
  static patomic<int> _threads_at_stage[MAX_STAGES];
  static patomic<int> _inplace_writers_at_stage[MAX_STAGES];

#endif  // CPPPARSER
};

#ifndef CPPPARSER
ALWAYS_INLINE void EpochManager::
enter(EpochParticipant &p) {
  if (p.depth++ == 0) {
    // Register before publishing the slot (slot is still 0 == quiescent here,
    // and no pointer has been loaded yet) so every reclaimer scan sees us.
    if (!p.registered) {
      register_participant(&p);
    }
    p.slot.store(_global_epoch.load(std::memory_order_acquire),
                 std::memory_order_release);
    // StoreLoad barrier: the slot store above must be visible before this
    // thread loads any published CycleData pointer.  Release/acquire does not
    // order a store ahead of a later load to a different location, so without
    // this fence (even on x86, whose store buffer reorders store->load) the
    // load could hoist above the slot store, letting a reclaimer see us
    // quiescent and free a pointer we are about to dereference.  Pairs with the
    // fence in observed_min_epoch_locked().  Outermost enter only (~per frame).
    // StoreLoad fence is provided for free by the seq_cst RMW below: on x86
    // LOCK XADD drains the store buffer and is a full barrier; on ARM,
    // seq_cst adds a DMB ISH after STLXR.  This subsumes the previous
    // patomic_thread_fence(seq_cst) which was redundant on x86.
    _active_cs_count.fetch_add(1, std::memory_order_seq_cst);
  }
}

ALWAYS_INLINE void EpochManager::
leave(EpochParticipant &p) {
  if (--p.depth == 0) {
    p.slot.store(0, std::memory_order_release);
    int prev = _active_cs_count.fetch_sub(1, std::memory_order_acq_rel);
    if (p.online) {
      // An "online" thread reclaims in throttled batches rather than once
      // per outermost section.  Otherwise a lone main thread doing unframed
      // writes would advance and reclaim the epoch on every single write.
      // When its work is instead nested in a frame/task epoch, the nested
      // sections never reach here, so this fires once, at the frame's own leave.
      if (_retired_count.load(std::memory_order_relaxed) >= checkpoint_threshold) {
        try_advance_epoch();
        try_reclaim(256);
      }
    } else if (prev == 1 &&
               _retired_count.load(std::memory_order_relaxed) > 0) {
      // Last thread out of a critical section: the pipeline is fully quiescent,
      // so drain retired CData. While any other thread is mid-frame this is
      // skipped, so the render path is untouched -- there poll()/cycle() drive
      // reclamation at frame boundaries instead.
      try_advance_epoch();
      try_reclaim(256);
    }
  }
}

ALWAYS_INLINE void EpochManager::
consider_reclaim(EpochParticipant &p) {
  // Skip mid-critical-section: our own slot would gate it anyway, and we must
  // not free a CycleData while a writer on this thread holds the cycler lock.
  if (p.depth != 0) {
    return;
  }
  // Throttle: a hot I/O loop may yield per buffer; poll()/cycle() are the
  // primary drivers, so an occasional nudge here is enough.
  if ((++p.reclaim_ticks & 7u) != 0) {
    return;
  }
  try_advance_epoch();
  try_reclaim(256);
}

ALWAYS_INLINE bool EpochManager::
inplace_allowed_hint(int stage) {
  // Relaxed; try_begin_inplace_write's recheck is the load-bearing one.
  if (stage < 0 || stage >= MAX_STAGES) {
    return false;
  }
  return _threads_at_stage[stage].load(std::memory_order_relaxed) <= 1;
}

ALWAYS_INLINE bool EpochManager::
try_begin_inplace_write(int stage) {
  if (stage < 0 || stage >= MAX_STAGES) {
    return false;
  }
  // Claim a slot, then re-check the count.  Either a concurrently-arriving
  // thread raised the count before our increment (we see >= 2 and back out),
  // or after it (its stage_occupancy_inc spin-drains on our +1).  So no thread
  // becomes an occupant of this stage while an in-place write here is live.
  _inplace_writers_at_stage[stage].fetch_add(1, std::memory_order_acq_rel);
  if (_threads_at_stage[stage].load(std::memory_order_acquire) >= 2) {
    _inplace_writers_at_stage[stage].fetch_sub(1, std::memory_order_release);
    return false;
  }
  return true;
}

ALWAYS_INLINE void EpochManager::
end_inplace_write(int stage) {
  if (stage < 0 || stage >= MAX_STAGES) {
    return;
  }
  _inplace_writers_at_stage[stage].fetch_sub(1, std::memory_order_release);
}
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
  ALWAYS_INLINE static bool try_begin_inplace_write(int) { return false; }
  ALWAYS_INLINE static void end_inplace_write(int) {}
  ALWAYS_INLINE static bool inplace_allowed_hint(int) { return false; }
};

#endif  // THREADED_PIPELINE

#endif  // EPOCHMANAGER_H
