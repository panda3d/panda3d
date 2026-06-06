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
