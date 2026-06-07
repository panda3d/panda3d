/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file epochManager.I
 * @author Maxwell Dreytser
 * @date 2026-05-17
 */

/**
 * Enters an EBR critical section on this participant. Depth-counted; only the
 * outermost enter registers and publishes the slot. The participant is passed
 * in to keep the hot path off the shared-library thread_local lookup (see
 * Thread::epoch_participant).
 */
ALWAYS_INLINE void EpochManager::
enter(EpochParticipant &p) {
  if (p.depth++ == 0) {
    if (!p.registered) {
      register_participant(&p);
    }
    p.slot.store(_global_epoch.load(std::memory_order_acquire),
                 std::memory_order_release);
    // StoreLoad fence: the slot store must be globally visible before this
    // thread loads any published pointer, or a reclaimer could see us quiescent
    // and free a pointer we are about to dereference.
    patomic_thread_fence(std::memory_order_seq_cst);
  }
}

/**
 * Leaves the critical section entered by enter().  On the outermost leave,
 * drains a batch if the retire backlog has grown past the throttle, bounding
 * in-frame growth. We have separate unbounded drains run at frame/tick boundaries.
 */
ALWAYS_INLINE void EpochManager::
leave(EpochParticipant &p) {
  if (--p.depth == 0) {
    p.slot.store(0, std::memory_order_release);
    if (_retired_count.load(std::memory_order_relaxed) >= checkpoint_threshold) {
      try_advance_epoch();
      try_reclaim(256);
    }
  }
}

/**
 * Opportunistic reclamation for the yield/sleep family, so threads that never
 * render or poll still make progress.  No-op mid-critical-section (our slot
 * would gate it, and a writer on this thread may hold the cycler lock) and
 * throttled otherwise.  Must use this thread's own participant.
 */
ALWAYS_INLINE void EpochManager::
consider_reclaim(EpochParticipant &p) {
  if (p.depth != 0) {
    return;
  }
  if ((++p.reclaim_ticks & 7u) != 0) {
    return;
  }
  try_advance_epoch();
  try_reclaim(256);
}

/**
 * Relaxed pre-check for the in-place fast path; try_begin_inplace_write() does
 * the load-bearing recheck.
 */
ALWAYS_INLINE bool EpochManager::
inplace_allowed_hint(int stage) {
  if (stage < 0 || stage >= MAX_STAGES) {
    return false;
  }
  return _threads_at_stage[stage].load(std::memory_order_relaxed) <= 1;
}

/**
 * Claims an in-place write at a stage, returning false (caller must
 * copy-on-write) if another thread occupies it.  Claims the slot, then rechecks
 * the occupant count: an arriving thread is either seen here, or its
 * stage_occupancy_inc() spin-drains on this claim, so no thread reads the stage
 * while an in-place write is live.
 */
ALWAYS_INLINE bool EpochManager::
try_begin_inplace_write(int stage) {
  if (stage < 0 || stage >= MAX_STAGES) {
    return false;
  }
  _inplace_writers_at_stage[stage].fetch_add(1, std::memory_order_acq_rel);
  if (_threads_at_stage[stage].load(std::memory_order_acquire) >= 2) {
    _inplace_writers_at_stage[stage].fetch_sub(1, std::memory_order_release);
    return false;
  }
  return true;
}

/**
 * Releases the claim taken by try_begin_inplace_write().
 */
ALWAYS_INLINE void EpochManager::
end_inplace_write(int stage) {
  if (stage < 0 || stage >= MAX_STAGES) {
    return;
  }
  _inplace_writers_at_stage[stage].fetch_sub(1, std::memory_order_release);
}
