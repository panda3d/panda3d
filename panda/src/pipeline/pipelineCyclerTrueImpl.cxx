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
  _dirty(0),
  _lock(this)
{
  if (_pipeline == nullptr) {
    _pipeline = Pipeline::get_render_pipeline();
  }

  _num_stages = _pipeline->get_num_stages();
  _data = new CycleDataNode[_num_stages];
  for (int i = 0; i < _num_stages; ++i) {
    _data[i]._cdata = initial_data;
  }

  _pipeline->add_cycler(this);
}

/**
 *
 */
PipelineCyclerTrueImpl::
PipelineCyclerTrueImpl(const PipelineCyclerTrueImpl &copy) :
  _pipeline(copy._pipeline),
  _dirty(0),
  _lock(this)
{
  ReMutexHolder holder(_lock);
  ReMutexHolder holder2(copy._lock);

  _num_stages = _pipeline->get_num_stages();
  nassertv(_num_stages == copy._num_stages);
  _data = new CycleDataNode[_num_stages];

  // It's no longer critically important that we preserve pointerwise
  // equivalence between different stages in the copy, but it doesn't cost
  // much and might be a little more efficient, so we do it anyway.
  typedef pmap<CycleData *, PT(CycleData) > Pointers;
  Pointers pointers;

  for (int i = 0; i < _num_stages; ++i) {
    PT(CycleData) &new_pt = pointers[copy._data[i]._cdata];
    if (new_pt == nullptr) {
      new_pt = copy._data[i]._cdata->make_copy();
    }
    _data[i]._cdata = new_pt.p();
  }

  _pipeline->add_cycler(this);
  if (copy._dirty) {
    _pipeline->add_dirty_cycler(this);
  }
}

/**
 *
 */
void PipelineCyclerTrueImpl::
operator = (const PipelineCyclerTrueImpl &copy) {
  ReMutexHolder holder1(_lock);
  ReMutexHolder holder2(copy._lock);
  nassertv(get_parent_type() == copy.get_parent_type());

  typedef pmap<CycleData *, PT(CycleData) > Pointers;
  Pointers pointers;

  for (int i = 0; i < _num_stages; ++i) {
    PT(CycleData) &new_pt = pointers[copy._data[i]._cdata];
    if (new_pt == nullptr) {
      new_pt = copy._data[i]._cdata->make_copy();
    }
    _data[i]._cdata = new_pt.p();
  }

  if (copy._dirty && !_dirty) {
    _pipeline->add_dirty_cycler(this);
  }
}

/**
 *
 */
PipelineCyclerTrueImpl::
~PipelineCyclerTrueImpl() {
  ReMutexHolder holder(_lock);

  _pipeline->remove_cycler(this);

  delete[] _data;
  _data = nullptr;
  _num_stages = 0;
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

#ifndef NDEBUG
  nassertd(pipeline_stage >= 0 && pipeline_stage < _num_stages) {
    _lock.release();
    return nullptr;
  }
#endif  // NDEBUG

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
      if (!_dirty && _num_stages != 1) {
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

#ifndef NDEBUG
  nassertd(pipeline_stage >= 0 && pipeline_stage < _num_stages) {
    _lock.release();
    return nullptr;
  }
#endif  // NDEBUG

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

      if (k >= 0 || pipeline_stage + 1 < _num_stages) {
        // Now we have differences between some of the data pointers, which
        // makes us "dirty".
        if (!_dirty) {
          _pipeline->add_dirty_cycler(this);
        }
      }

    } else if (k >= 0 && force_to_0) {
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
  PT(CycleData) last_val;
  last_val.swap(_data[_num_stages - 1]._cdata);
  last_val->node_unref_only();

  nassertr(_lock.debug_is_locked(), last_val);
  nassertr(_dirty, last_val);

  int i;
  for (i = _num_stages - 1; i > 0; --i) {
    nassertr(_data[i]._writes_outstanding == 0, last_val);
    _data[i]._cdata = _data[i - 1]._cdata;
  }

  for (i = 1; i < _num_stages; ++i) {
    if (_data[i]._cdata != _data[i - 1]._cdata) {
      // Still dirty.
      return last_val;
    }
  }

  // No longer dirty.
  _dirty = 0;
  return last_val;
}

/**
 * Changes the number of stages in the cycler.  This is only called from
 * Pipeline::set_num_stages();
 */
void PipelineCyclerTrueImpl::
set_num_stages(int num_stages) {
  nassertv(_lock.debug_is_locked());

  if (num_stages <= _num_stages) {
    // Don't bother to reallocate the array smaller; we just won't use the
    // rest of the array.
    for (int i = _num_stages; i < num_stages; ++i) {
      nassertv(_data[i]._writes_outstanding == 0);
      _data[i]._cdata.clear();
    }

    _num_stages = num_stages;


  } else {
    // To increase the array, we must reallocate it larger.
    CycleDataNode *new_data = new CycleDataNode[num_stages];
    int i;
    for (i = 0; i < _num_stages; ++i) {
      nassertv(_data[i]._writes_outstanding == 0);
      new_data[i]._cdata = _data[i]._cdata;
    }
    for (i = _num_stages; i < num_stages; ++i) {
      new_data[i]._cdata = _data[_num_stages - 1]._cdata;
    }
    delete[] _data;

    _num_stages = num_stages;
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
