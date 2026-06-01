/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pipelineCyclerTrueImpl.cxx
 * @author drose
 * @date 2006-01-31
 */

#include "pipelineCyclerTrueImpl.h"
#include "epochManager.h"

#ifdef THREADED_PIPELINE

#include "config_pipeline.h"
#include "pipeline.h"

/**
 *
 */
PipelineCyclerTrueImpl::
PipelineCyclerTrueImpl(CycleData *initial_data, Pipeline *pipeline) :
  _pipeline(pipeline),
  _lock(this)
{
  clear_dirty();

  if (_pipeline == nullptr) {
    _pipeline = Pipeline::get_render_pipeline();
  }

  int num_stages = _pipeline->get_num_stages();
  if (num_stages == 1) {
    _single_data.install(initial_data);
    _data = &_single_data;
  }
  else {
    _data = new CycleDataNode[num_stages];
    _data[0]._num_stages = num_stages;

    for (int i = 0; i < num_stages; ++i) {
      _data[i].install(initial_data);
    }
  }

  _pipeline->add_cycler(this);
}

/**
 *
 */
PipelineCyclerTrueImpl::
PipelineCyclerTrueImpl(const PipelineCyclerTrueImpl &copy) :
  _pipeline(copy._pipeline),
  _lock(this)
{
  clear_dirty();

  ReMutexHolder holder(_lock);
  ReMutexHolder holder2(copy._lock);

  int num_stages = _pipeline->get_num_stages();
  nassertv(num_stages == copy.get_num_stages());

  if (num_stages == 1) {
    // current_locked, not the published pointer: if `copy` has a COW in flight
    // (e.g. we are being copied from inside an open CDWriter on this thread),
    // the live data is in its _pending.
    CycleData *src = copy.current_locked(0);
    _single_data.install(src->make_copy());
    _data = &_single_data;
  }
  else {
    _data = new CycleDataNode[num_stages];
    _data[0]._num_stages = num_stages;

    // It's no longer critically important that we preserve pointerwise
    // equivalence between different stages in the copy, but it doesn't cost
    // much and might be a little more efficient, so we do it anyway.
    typedef pmap<CycleData *, PT(CycleData) > Pointers;
    Pointers pointers;

    for (int i = 0; i < num_stages; ++i) {
      CycleData *src = copy.current_locked(i);
      PT(CycleData) &new_pt = pointers[src];
      if (new_pt == nullptr) {
        new_pt = src->make_copy();
      }
      _data[i].install(new_pt.p());
    }
  }

  _pipeline->add_cycler(this, copy.is_dirty());
}

/**
 *
 */
void PipelineCyclerTrueImpl::
operator = (const PipelineCyclerTrueImpl &copy) {
  ReMutexHolder holder1(_lock);
  ReMutexHolder holder2(copy._lock);
  nassertv(get_parent_type() == copy.get_parent_type());

  // Build the new published pointers, then retire the old ones to EBR -- the
  // release_write_stage publish path, applied to every stage at once.
  if (_data == &_single_data) {
    CycleData *src = copy.current_locked(0);
    CycleData *new_data = src->make_copy();
    new_data->node_ref();
    CycleData *old = _single_data._cdata.exchange(new_data, std::memory_order_release);
    EpochManager::retire(old);
    nassertv(!copy.is_dirty());
  }
  else {
    int num_stages = _data[0]._num_stages;

    typedef pmap<CycleData *, PT(CycleData) > Pointers;
    Pointers pointers;

    for (int i = 0; i < num_stages; ++i) {
      CycleData *src = copy.current_locked(i);
      PT(CycleData) &new_pt = pointers[src];
      if (new_pt == nullptr) {
        new_pt = src->make_copy();
      }
      CycleData *new_data = new_pt.p();
      new_data->node_ref();
      CycleData *old = _data[i]._cdata.exchange(new_data, std::memory_order_release);
      EpochManager::retire(old);
    }

    if (copy.is_dirty() && !is_dirty()) {
      _pipeline->add_dirty_cycler(this);
    }
  }
}

/**
 *
 */
PipelineCyclerTrueImpl::
~PipelineCyclerTrueImpl() {
  ReMutexHolder holder(_lock);

  _pipeline->remove_cycler(this);

  if (_data != &_single_data) {
    delete[] _data;
  }
  _data = nullptr;
}

/**
 * Returns a pointer suitable for writing to the nth stage of the pipeline.
 * This is for special applications that need to update the entire pipeline at
 * once (for instance, to remove an invalid pointer). This pointer should
 * later be released with release_write_stage().
 */
CycleData *PipelineCyclerTrueImpl::
write_stage(int pipeline_stage, Thread *current_thread) {
  _lock.acquire(current_thread);

#ifdef _DEBUG
  nassertd(pipeline_stage >= 0 && pipeline_stage < get_num_stages()) {
    _lock.release();
    return nullptr;
  }
#endif

  CycleDataNode &slot = _data[pipeline_stage];

  if (slot._writes_outstanding == 0) {
    // In-place fast path: mutate the published CData directly when no reader
    // can observe it -- this stage's CData is unshared (stage_unshared) and the
    // writer is the sole occupant of its own stage (try_begin_inplace_write).
    // It must be the writer's OWN stage: the occupancy interlock only certifies
    // "no other reader at S" for S's sole occupant, so cross-stage writes COW.
    // `_pending == nullptr` with writes outstanding marks in-place mode.
    if (!pipeline_always_cow &&
        pipeline_stage == current_thread->get_pipeline_stage() &&
        EpochManager::inplace_allowed_hint(pipeline_stage) &&
        stage_unshared(pipeline_stage) &&
        EpochManager::try_begin_inplace_write(pipeline_stage)) {
      slot._pending = nullptr;
      ++slot._writes_outstanding;
      return slot._cdata.load(std::memory_order_relaxed);
    }
    // COW path: copy, mutate, then publish + retire on commit.  The only path
    // safe under concurrent readers from other threads or stages.
    CycleData *current = current_locked(pipeline_stage);
    slot._pending = current->make_copy();
    // node_ref the copy now, not at publish: reads on this thread are steered
    // to _pending (read-your-writes), so a reader that ref()/unref_delete()s it
    // must not be able to free the writer's live copy.
    slot._pending->node_ref();
    // Steer this thread's own lock-free reads to _pending until we publish.
    slot._write_thread.store(current_thread, std::memory_order_release);
    if (pipeline_cat.is_debug()) {
      pipeline_cat.debug()
        << "Copy-on-write a: " << current << " becomes "
        << slot._pending << "\n";
    }
    if (!is_dirty() && _data != &_single_data) {
      _pipeline->add_dirty_cycler(this);
    }
  }

  ++slot._writes_outstanding;
  // _pending may be nullptr (in-place mode) or the new COW copy.
  return slot._pending != nullptr
      ? slot._pending
      : slot._cdata.load(std::memory_order_relaxed);
}

/**
 * Returns true if the indicated stage's published CData pointer is not
 * shared with any other stage's slot.  Mutating a stage's CData in place
 * is only safe when it is unshared, because a reader at another stage
 * reads that stage's slot and would observe a torn mutation if the
 * pointer were shared.  Called with the cycler lock held.
 */
bool PipelineCyclerTrueImpl::
stage_unshared(int pipeline_stage) const {
  int num_stages = get_num_stages();
  if (num_stages == 1) {
    return true;
  }
  CycleData *p = _data[pipeline_stage]._cdata.load(std::memory_order_relaxed);
  for (int i = 0; i < num_stages; ++i) {
    if (i != pipeline_stage &&
        _data[i]._cdata.load(std::memory_order_relaxed) == p) {
      return false;
    }
  }
  return true;
}

/**
 * This special variant on write_stage() will automatically propagate changes
 * back to upstream pipeline stages.  See write_upstream().
 */
CycleData *PipelineCyclerTrueImpl::
write_stage_upstream(int pipeline_stage, bool force_to_0, Thread *current_thread) {
  // Under always-COW EBR, the old trick of pointer-assigning the in-flight
  // write into each upstream stage would expose those stages' readers to a
  // half-mutated CycleData.  A plain write_stage() suffices; Pipeline::cycle()
  // propagates downstream next frame, so force_to_0 becomes a one-cycle delay.
  (void)force_to_0;
  return write_stage(pipeline_stage, current_thread);
}

/**
 * Cycles the data between frames.  This is only called from
 * Pipeline::cycle(), and presumably it is only called if the cycler is
 * "dirty".
 *
 * At the conclusion of this method, the cycler should clear its dirty flag if
 * it is no longer "dirty"--that is, if all of the pipeline pointers are the
 * same.
 *
 * The return value is the CycleData pointer which fell off the end of the
 * cycle.  If this is allowed to destruct immediately, there may be side-
 * effects that cascade through the system, so the caller may choose to hold
 * the pointer until it can safely be released later.
 */
void PipelineCyclerTrueImpl::
cycle() {
  int num_stages = get_num_stages();
  nassertv(_lock.debug_is_locked());
  nassertv(is_dirty());

  // Rotate stage i-1 -> i (high to low).  Each step node_refs the destination
  // slot, exchanges, and yields the displaced pointer.  Every displaced
  // pointer but the last is double-counted (stage i+1 already node_ref'd the
  // same pointer last iteration), so it is unref'd immediately; the last (which
  // falls off the end) goes to EBR so concurrent CDReaders observing it stay
  // valid.
  for (int i = num_stages - 1; i > 0; --i) {
    nassertv(_data[i]._writes_outstanding == 0);
    CycleData *incoming = _data[i - 1]._cdata.load(std::memory_order_relaxed);
    if (incoming != nullptr) {
      incoming->node_ref();
    }
    CycleData *displaced =
      _data[i]._cdata.exchange(incoming, std::memory_order_release);
    if (i == num_stages - 1) {
      EpochManager::retire(displaced);
    } else if (displaced != nullptr) {
      node_unref_delete<CycleData>(displaced);
    }
  }

  bool any_diff = false;
  for (int i = 1; i < num_stages; ++i) {
    if (_data[i]._cdata.load(std::memory_order_relaxed) !=
        _data[i - 1]._cdata.load(std::memory_order_relaxed)) {
      any_diff = true;
      break;
    }
  }
  if (!any_diff) {
    clear_dirty();
  }
}

/**
 * Changes the number of stages in the cycler.  This is only called from
 * Pipeline::set_num_stages();
 */
void PipelineCyclerTrueImpl::
set_num_stages(int num_stages) {
  nassertv(_lock.debug_is_locked());

  if (_data == &_single_data) {
    // We've got only 1 stage.  Allocate an array.
    if (num_stages > 1) {
      nassertv(_single_data._writes_outstanding == 0);

      CycleDataNode *new_data = new CycleDataNode[num_stages];
      new_data[0]._num_stages = num_stages;

      // Transfer the single slot to slot 0 of the new array without
      // touching node_ref counts.
      CycleData *p = _single_data._cdata.exchange(nullptr, std::memory_order_acq_rel);
      new_data[0]._cdata.store(p, std::memory_order_release);

      // Replicate to the other stages, bumping node_ref per replica.
      for (int i = 1; i < num_stages; ++i) {
        if (p != nullptr) p->node_ref();
        new_data[i]._cdata.store(p, std::memory_order_release);
      }
      _data = new_data;
    }
  }
  else if (num_stages == 1) {
    // Deallocate the array, since we're back to one stage.  Keep stage 0;
    // drop the rest.
    if (_data != &_single_data) {
      nassertv(_data[0]._writes_outstanding == 0);
      CycleData *keep = _data[0]._cdata.exchange(nullptr, std::memory_order_acq_rel);
      // Drop ALL the other stages' refs.
      int old_stages = _data[0]._num_stages;
      for (int i = 1; i < old_stages; ++i) {
        CycleData *p = _data[i]._cdata.exchange(nullptr, std::memory_order_acq_rel);
        if (p != nullptr) {
          node_unref_delete<CycleData>(p);
        }
      }
      _single_data._cdata.store(keep, std::memory_order_release);
      CycleDataNode *old_array = _data;
      _data = &_single_data;
      delete[] old_array;
    }
  }
  else if (num_stages <= _data[0]._num_stages) {
    // Don't bother to reallocate the array smaller; we just won't use the
    // rest of the array.  Drop the dropped stages' refs.
    int old_stages = _data[0]._num_stages;
    for (int i = num_stages; i < old_stages; ++i) {
      nassertv(_data[i]._writes_outstanding == 0);
      CycleData *p = _data[i]._cdata.exchange(nullptr, std::memory_order_acq_rel);
      if (p != nullptr) {
        node_unref_delete<CycleData>(p);
      }
    }

    _data[0]._num_stages = num_stages;
  }
  else {
    // To increase the array, we must reallocate it larger.
    int old_stages = _data[0]._num_stages;
    CycleDataNode *new_data = new CycleDataNode[num_stages];
    new_data[0]._num_stages = num_stages;

    int i;
    for (i = 0; i < old_stages; ++i) {
      nassertv(_data[i]._writes_outstanding == 0);
      CycleData *p = _data[i]._cdata.exchange(nullptr, std::memory_order_acq_rel);
      new_data[i]._cdata.store(p, std::memory_order_release);
    }
    CycleData *fill = new_data[old_stages - 1]._cdata.load(std::memory_order_relaxed);
    for (i = old_stages; i < num_stages; ++i) {
      if (fill != nullptr) fill->node_ref();
      new_data[i]._cdata.store(fill, std::memory_order_release);
    }
    delete[] _data;
    _data = new_data;
  }
}

#ifdef DEBUG_THREADS
/**
 *
 */
void PipelineCyclerTrueImpl::CyclerMutex::
output(std::ostream &out) const {
  out << "CyclerMutex ";
  _cycler->cheat()->output(out);
}
#endif  // DEBUG_THREADS

#endif  // THREADED_PIPELINE
