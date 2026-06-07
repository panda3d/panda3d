/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file epochManager.cxx
 */

#include "epochManager.h"

#ifdef THREADED_PIPELINE

#include "cycleData.h"
#include "thread.h"
#include "pmutex.h"
#include "mutexHolder.h"

#include <algorithm>

// 0 is reserved for "quiescent".  Start the counter at 1 so a thread that
// has never been in a CS (slot == 0) is treated as not blocking advance.
patomic<uint64_t> EpochManager::_global_epoch{1};

patomic<int> EpochManager::_active_cs_count{0};
patomic<size_t> EpochManager::_retired_count{0};

namespace {
  struct Retired {
    uint64_t epoch;
    CycleData *cd;
  };
  struct EbrRegistry {
    Mutex registry_lock;
    EpochParticipant *head = nullptr;
    Mutex retired_lock;
    pvector<Retired> retired;
  };
  EbrRegistry *get_registry() {
    static EbrRegistry *r = new EbrRegistry();
    return r;
  }
}

static thread_local bool tl_in_reclaim = false;

bool EpochManager::
is_reclaiming() {
  return tl_in_reclaim;
}

EpochParticipant::
~EpochParticipant() {
  if (registered) {
    EpochManager::unregister_participant(this);
  }
}

void EpochManager::
register_participant(EpochParticipant *p) {
  EbrRegistry *r = get_registry();
  MutexHolder hold(r->registry_lock);
  p->next = r->head;
  r->head = p;
  p->registered = true;
}

void EpochManager::
unregister_participant(EpochParticipant *p) {
  EbrRegistry *r = get_registry();
  MutexHolder hold(r->registry_lock);
  EpochParticipant **pp = &r->head;
  while (*pp != nullptr && *pp != p) {
    pp = &(*pp)->next;
  }
  if (*pp == p) {
    *pp = p->next;
  }
  p->next = nullptr;
  p->registered = false;
}

/**
 * Indicate a thread allowed to open writers with no enclosing epoch (the main
 * thread). It still publishes its slot per critical section, so an idle one
 * never pins the reclaim floor; what differs is that reclamation on its
 * behalf is throttled (see leave()) rather than run on every outermost leave,
 * which is what minimizes the performance impact.
 * @param p
 */
void EpochManager::
go_online(EpochParticipant &p) {
  p.online = true;
}

patomic<int> EpochManager::_threads_at_stage[EpochManager::MAX_STAGES];
patomic<int> EpochManager::_inplace_writers_at_stage[EpochManager::MAX_STAGES];

// On raising a stage to 2+ occupants, drain in-flight in-place writers that
// slipped in before our increment became visible, so an arriving thread can't
// read a stage while an in-place write there is live.
void EpochManager::
stage_occupancy_inc(int stage) {
  if (stage < 0 || stage >= MAX_STAGES) {
    return;
  }
  int n = _threads_at_stage[stage].fetch_add(1, std::memory_order_acq_rel) + 1;
  if (n >= 2) {
    while (_inplace_writers_at_stage[stage].load(std::memory_order_acquire) > 0) {
      Thread::relax();
    }
  }
}

void EpochManager::
stage_occupancy_dec(int stage) {
  if (stage < 0 || stage >= MAX_STAGES) {
    return;
  }
  _threads_at_stage[stage].fetch_sub(1, std::memory_order_release);
}

void EpochManager::
retire(CycleData *cd) {
  if (cd == nullptr) {
    return;
  }
  uint64_t e = _global_epoch.load(std::memory_order_acquire);
  {
    EbrRegistry *r = get_registry();
    MutexHolder hold(r->retired_lock);
    r->retired.push_back({e, cd});
  }
  _retired_count.fetch_add(1, std::memory_order_relaxed);
}

// Min slot among in-CS participants (slot != 0), or UINT64_MAX if all
// quiescent.  Caller holds the registry lock.
static uint64_t observed_min_epoch_locked(EpochParticipant *head) {
  // StoreLoad barrier pairing with the reader's enter() fence: order the
  // reclaimer's prior global-epoch read and retire stamping ahead of these slot
  // loads, so a reader that has already published its slot is never observed
  // here as quiescent while it holds a pointer we are about to free.  Cold path
  // (per advance/reclaim, ~once per frame), so the full fence costs nothing.
  patomic_thread_fence(std::memory_order_seq_cst);
  uint64_t min_e = ~uint64_t(0);
  for (EpochParticipant *p = head; p != nullptr; p = p->next) {
    uint64_t e = p->slot.load(std::memory_order_acquire);
    if (e != 0 && e < min_e) {
      min_e = e;
    }
  }
  return min_e;
}

void EpochManager::
try_advance_epoch() {
  uint64_t cur = _global_epoch.load(std::memory_order_relaxed);
  uint64_t min_observed;
  {
    EbrRegistry *r = get_registry();
    MutexHolder hold(r->registry_lock);
    min_observed = observed_min_epoch_locked(r->head);
  }
  // If everyone in a CS has observed `cur`, mint a new epoch; entries stamped
  // `cur - 1` or older then become reclaimable.
  if (min_observed >= cur) {
    _global_epoch.fetch_add(1, std::memory_order_acq_rel);
  }
}

size_t EpochManager::
try_reclaim(size_t budget) {
  if (budget == 0) {
    return 0;
  }
  uint64_t min_observed;
  {
    EbrRegistry *r = get_registry();
    MutexHolder hold(r->registry_lock);
    min_observed = observed_min_epoch_locked(r->head);
  }
  // All quiescent -> the floor is the current epoch (free anything older).
  if (min_observed == ~uint64_t(0)) {
    min_observed = _global_epoch.load(std::memory_order_acquire);
  }

  // Collect into a local vector; free outside _retired_lock (free can take
  // other locks).
  pvector<CycleData *> to_free;
  {
    EbrRegistry *r = get_registry();
    MutexHolder hold(r->retired_lock);
    auto write = r->retired.begin();
    for (auto read = r->retired.begin(); read != r->retired.end(); ++read) {
      if (read->epoch < min_observed && to_free.size() < budget) {
        to_free.push_back(read->cd);
      } else {
        if (write != read) {
          *write = *read;
        }
        ++write;
      }
    }
    r->retired.erase(write, r->retired.end());
  }

  if (!to_free.empty()) {
    _retired_count.fetch_sub(to_free.size(), std::memory_order_relaxed);
  }
  bool prev_in_reclaim = tl_in_reclaim;
  tl_in_reclaim = true;
  for (CycleData *cd : to_free) {
    // Undoes the node_ref() the cycler did on publish/install; deletes at zero.
    node_unref_delete<CycleData>(cd);
  }
  tl_in_reclaim = prev_in_reclaim;
  return to_free.size();
}

void EpochManager::
register_thread(Thread *t, int stage) {
  if (t == nullptr) {
    return;
  }
  // Stage occupancy only (for the in-place fast path); epoch participation is
  // separate and keyed to the OS thread.
  stage_occupancy_inc(stage);
}

void EpochManager::
thread_stage_changed(int old_stage, int new_stage) {
  if (old_stage == new_stage) {
    return;
  }
  // Raise the new stage (with its drain) before lowering the old, so the
  // moving thread can't read the new stage while an in-place write is live.
  stage_occupancy_inc(new_stage);
  stage_occupancy_dec(old_stage);
}

void EpochManager::
unregister_thread(Thread *t, int stage) {
  if (t == nullptr) {
    return;
  }
  stage_occupancy_dec(stage);
}

uint64_t EpochManager::
get_global_epoch() {
  return _global_epoch.load(std::memory_order_acquire);
}

size_t EpochManager::
get_retired_count() {
  EbrRegistry *r = get_registry();
  MutexHolder hold(r->retired_lock);
  return r->retired.size();
}

int EpochManager::
get_threads_at_stage(int stage) {
  if (stage < 0 || stage >= MAX_STAGES) {
    return -1;
  }
  return _threads_at_stage[stage].load(std::memory_order_relaxed);
}

#endif  // THREADED_PIPELINE
