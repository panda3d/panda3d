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
    _single_data._cdata = initial_data;
    _data = &_single_data;
  }
  else {
    _data = new CycleDataNode[num_stages];
    _data[0]._num_stages = num_stages;

    for (int i = 0; i < num_stages; ++i) {
      _data[i]._cdata = initial_data;
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
    _single_data._cdata = copy._single_data._cdata->make_copy();
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
      PT(CycleData) &new_pt = pointers[copy._data[i]._cdata];
      if (new_pt == nullptr) {
        new_pt = copy._data[i]._cdata->make_copy();
      }
      _data[i]._cdata = new_pt.p();
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

  if (_data == &_single_data) {
    _single_data._cdata = copy._single_data._cdata->make_copy();
    nassertv(!copy.is_dirty());
  }
  else {
    int num_stages = _data[0]._num_stages;

    typedef pmap<CycleData *, PT(CycleData) > Pointers;
    Pointers pointers;

    for (int i = 0; i < num_stages; ++i) {
      PT(CycleData) &new_pt = pointers[copy._data[i]._cdata];
      if (new_pt == nullptr) {
        new_pt = copy._data[i]._cdata->make_copy();
      }
      _data[i]._cdata = new_pt.p();
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

  CycleData *old_data = _data[pipeline_stage]._cdata;

  // We only perform copy-on-write if this is the first CycleData requested
  // for write mode from this thread.  (We will never have outstanding writes
  // for multiple threads, because we hold the CyclerMutex during the entire
  // lifetime of write() .. release()).
  if (_data[pipeline_stage]._writes_outstanding == 0) {
    // Only the node reference count is considered an important count for
    // copy-on-write purposes.  A standard reference of other than 1 just
    // means that some code (other that the PipelineCycler) has a pointer,
    // which is safe to modify.
    if (old_data->get_node_ref_count() != 1) {
      // Copy-on-write.
      _data[pipeline_stage]._cdata = old_data->make_copy();
      if (pipeline_cat.is_debug()) {
        pipeline_cat.debug()
          << "Copy-on-write a: " << old_data << " becomes "
          << _data[pipeline_stage]._cdata << "\n";
        // nassertr(false, NULL);
      }

      // Now we have differences between some of the data pointers, so we're
      // "dirty".  Mark it so.
      if (!is_dirty() && _data != &_single_data) {
        _pipeline->add_dirty_cycler(this);
      }
    }
  }

  ++(_data[pipeline_stage]._writes_outstanding);
  return _data[pipeline_stage]._cdata;
}

/**
 * This special variant on write_stage() will automatically propagate changes
 * back to upstream pipeline stages.  See write_upstream().
 */
CycleData *PipelineCyclerTrueImpl::
write_stage_upstream(int pipeline_stage, bool force_to_0, Thread *current_thread) {
  _lock.acquire(current_thread);

#ifdef _DEBUG
  nassertd(pipeline_stage >= 0 && pipeline_stage < get_num_stages()) {
    _lock.release();
    return nullptr;
  }
#endif

  CycleData *old_data = _data[pipeline_stage]._cdata;

  if (old_data->get_ref_count() != 1 || force_to_0) {
    // Count the number of references before the current stage, and the number
    // of references remaining other than those.
    int external_count = old_data->get_ref_count() - 1;
    int k = pipeline_stage - 1;
    while (k >= 0 && _data[k]._cdata == old_data) {
      --k;
      --external_count;
    }

    // We only perform copy-on-write if this is the first CycleData requested
    // for write mode from this thread.  (We will never have outstanding
    // writes for multiple threads, because we hold the CyclerMutex during the
    // entire lifetime of write() .. release()).
    if (external_count > 0 && _data[pipeline_stage]._writes_outstanding == 0) {
      // There are references other than the ones before this stage in the
      // pipeline; perform a copy-on-write.
      PT(CycleData) new_data = old_data->make_copy();
      if (pipeline_cat.is_debug()) {
        pipeline_cat.debug()
          << "Copy-on-write b: " << old_data << " becomes "
          << new_data << "\n";
        // nassertr(false, NULL);
      }

      k = pipeline_stage - 1;
      while (k >= 0 && (_data[k]._cdata == old_data || force_to_0)) {
        nassertr(_data[k]._writes_outstanding == 0, nullptr);
        _data[k]._cdata = new_data.p();
        --k;
      }

      _data[pipeline_stage]._cdata = new_data;

      if (k >= 0 || pipeline_stage + 1 < get_num_stages()) {
        // Now we have differences between some of the data pointers, which
        // makes us "dirty".
        if (!is_dirty()) {
          _pipeline->add_dirty_cycler(this);
        }
      }
    }
    else if (k >= 0 && force_to_0) {
      // There are no external pointers, so no need to copy-on-write, but the
      // current pointer doesn't go all the way back.  Make it do so.
      while (k >= 0) {
        nassertr(_data[k]._writes_outstanding == 0, nullptr);
        _data[k]._cdata = old_data;
        --k;
      }
    }
  }

  ++(_data[pipeline_stage]._writes_outstanding);
  return _data[pipeline_stage]._cdata;
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
PT(CycleData) PipelineCyclerTrueImpl::
cycle() {
  // This trick moves an NPT into a PT without unnecessarily incrementing and
  // subsequently decrementing the regular reference count.
  int num_stages = get_num_stages();
  PT(CycleData) last_val;
  last_val.swap(_data[num_stages - 1]._cdata);
  last_val->node_unref_only();

  nassertr(_lock.debug_is_locked(), last_val);
  nassertr(is_dirty(), last_val);

  int i;
  for (i = num_stages - 1; i > 0; --i) {
    nassertr(_data[i]._writes_outstanding == 0, last_val);
    _data[i]._cdata = _data[i - 1]._cdata;
  }

  for (i = 1; i < num_stages; ++i) {
    if (_data[i]._cdata != _data[i - 1]._cdata) {
      // Still dirty.
      return last_val;
    }
  }

  // No longer dirty.
  clear_dirty();
  return last_val;
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
      new_data[0]._cdata = std::move(_single_data._cdata);
      _single_data._cdata.clear();

      for (int i = 1; i < num_stages; ++i) {
        new_data[i]._cdata = new_data[0]._cdata;
      }
      _data = new_data;
    }
  }
  else if (num_stages == 1) {
    // Deallocate the array, since we're back to one stage.
    if (_data != &_single_data) {
      nassertv(_data[0]._writes_outstanding == 0);
      _single_data._cdata = std::move(_data[0]._cdata);
      _data = &_single_data;
      delete[] _data;
    }
  }
  else if (num_stages <= _data[0]._num_stages) {
    // Don't bother to reallocate the array smaller; we just won't use the
    // rest of the array.
    int old_stages = _data[0]._num_stages;
    for (int i = old_stages; i < num_stages; ++i) {
      nassertv(_data[i]._writes_outstanding == 0);
      _data[i]._cdata.clear();
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
      new_data[i]._cdata = _data[i]._cdata;
    }
    for (i = old_stages; i < num_stages; ++i) {
      new_data[i]._cdata = _data[old_stages - 1]._cdata;
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
