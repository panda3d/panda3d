// Filename: pipelineCyclerTrueImpl.cxx
// Created by:  drose (31Jan06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "pipelineCyclerTrueImpl.h"

#ifdef THREADED_PIPELINE

#include "config_pipeline.h"
#include "pipeline.h"

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PipelineCyclerTrueImpl::
PipelineCyclerTrueImpl(CycleData *initial_data, Pipeline *pipeline) :
  _pipeline(pipeline),
  _dirty(false),
  _lock(this)
{
  if (_pipeline == (Pipeline *)NULL) {
    _pipeline = Pipeline::get_render_pipeline();
  }

  _num_stages = _pipeline->get_num_stages();
  _data = new NPT(CycleData)[_num_stages];
  for (int i = 0; i < _num_stages; ++i) {
    _data[i] = initial_data;
  }

  _pipeline->add_cycler(this);
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PipelineCyclerTrueImpl::
PipelineCyclerTrueImpl(const PipelineCyclerTrueImpl &copy) :
  _pipeline(copy._pipeline),
  _dirty(false),
  _lock(this)
{
  ReMutexHolder holder(_lock);
  ReMutexHolder holder2(copy._lock);
  
  _num_stages = _pipeline->get_num_stages();
  nassertv(_num_stages == copy._num_stages);
  _data = new NPT(CycleData)[_num_stages];
  
  // It's no longer critically important that we preserve pointerwise
  // equivalence between different stages in the copy, but it doesn't
  // cost much and might be a little more efficient, so we do it
  // anyway.
  typedef pmap<CycleData *, PT(CycleData) > Pointers;
  Pointers pointers;
  
  for (int i = 0; i < _num_stages; ++i) {
    PT(CycleData) &new_pt = pointers[copy._data[i]];
    if (new_pt == NULL) {
      new_pt = copy._data[i]->make_copy();
    }
    _data[i] = new_pt.p();
  }

  _pipeline->add_cycler(this);
  if (copy._dirty) {
    _pipeline->add_dirty_cycler(this);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::Copy Assignment
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PipelineCyclerTrueImpl::
operator = (const PipelineCyclerTrueImpl &copy) {
  ReMutexHolder holder1(_lock);
  ReMutexHolder holder2(copy._lock);
  nassertv(get_parent_type() == copy.get_parent_type());

  typedef pmap<CycleData *, PT(CycleData) > Pointers;
  Pointers pointers;

  for (int i = 0; i < _num_stages; ++i) {
    PT(CycleData) &new_pt = pointers[copy._data[i]];
    if (new_pt == NULL) {
      new_pt = copy._data[i]->make_copy();
    }
    _data[i] = new_pt.p();
  }

  if (copy._dirty && !_dirty) {
    _pipeline->add_dirty_cycler(this);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PipelineCyclerTrueImpl::
~PipelineCyclerTrueImpl() {
  ReMutexHolder holder(_lock);

  _pipeline->remove_cycler(this);

  delete[] _data;
  _data = NULL;
  _num_stages = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::write_stage
//       Access: Public
//  Description: Returns a pointer suitable for writing to the nth
//               stage of the pipeline.  This is for special
//               applications that need to update the entire pipeline
//               at once (for instance, to remove an invalid pointer).
//               This pointer should later be released with
//               release_write_stage().
////////////////////////////////////////////////////////////////////
CycleData *PipelineCyclerTrueImpl::
write_stage(int pipeline_stage, Thread *current_thread) {
  _lock.acquire(current_thread);

#ifndef NDEBUG
  nassertd(pipeline_stage >= 0 && pipeline_stage < _num_stages) {
    _lock.release();
    return NULL;
  }
#endif  // NDEBUG

  CycleData *old_data = _data[pipeline_stage];

  // Only the node reference count is considered an important count
  // for copy-on-write purposes.  A standard reference of other than 1
  // just means that some code (other that the PipelineCycler) has a
  // pointer, which is safe to modify.
  if (old_data->get_node_ref_count() != 1) {
    // Copy-on-write.
    _data[pipeline_stage] = old_data->make_copy();

    // Now we have differences between some of the data pointers, so
    // we're "dirty".  Mark it so.
    if (!_dirty && _num_stages != 1) {
      _pipeline->add_dirty_cycler(this);
    }
  }

  return _data[pipeline_stage];
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::write_stage_upstream
//       Access: Public
//  Description: This special variant on write_stage() will
//               automatically propagate changes back to upstream
//               pipeline stages.  See write_upstream().
////////////////////////////////////////////////////////////////////
CycleData *PipelineCyclerTrueImpl::
write_stage_upstream(int pipeline_stage, bool force_to_0, Thread *current_thread) {
  _lock.acquire(current_thread);

#ifndef NDEBUG
  nassertd(pipeline_stage >= 0 && pipeline_stage < _num_stages) {
    _lock.release();
    return NULL;
  }
#endif  // NDEBUG

  CycleData *old_data = _data[pipeline_stage];

  if (old_data->get_ref_count() != 1 || force_to_0) {
    // Count the number of references before the current stage, and
    // the number of references remaining other than those.
    int external_count = old_data->get_ref_count() - 1;
    int k = pipeline_stage - 1;
    while (k >= 0 && _data[k] == old_data) {
      --k;
      --external_count;
    }
    
    if (external_count > 0) {
      // There are references other than the ones before this stage in
      // the pipeline; perform a copy-on-write.
      PT(CycleData) new_data = old_data->make_copy();
      
      k = pipeline_stage - 1;
      while (k >= 0 && (_data[k] == old_data || force_to_0)) {
        _data[k] = new_data.p();
        --k;
      }
      
      _data[pipeline_stage] = new_data;
      
      if (k >= 0 || pipeline_stage + 1 < _num_stages) {
        // Now we have differences between some of the data pointers,
        // which makes us "dirty".
        if (!_dirty) {
          _pipeline->add_dirty_cycler(this);
        }
      }

    } else if (k >= 0 && force_to_0) {
      // There are no external pointers, so no need to copy-on-write,
      // but the current pointer doesn't go all the way back.  Make it
      // do so.
      while (k >= 0) {
        _data[k] = old_data;
        --k;
      }
    }
  }

  return _data[pipeline_stage];
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::cycle
//       Access: Private
//  Description: Cycles the data between frames.  This is only called
//               from Pipeline::cycle(), and presumably it is only
//               called if the cycler is "dirty".
//
//               At the conclusion of this method, the cycler should
//               clear its dirty flag if it is no longer "dirty"--that
//               is, if all of the pipeline pointers are the same.
//
//               The return value is the CycleData pointer which fell
//               off the end of the cycle.  If this is allowed to
//               destruct immediately, there may be side-effects that
//               cascade through the system, so the caller may choose
//               to hold the pointer until it can safely be released
//               later.
////////////////////////////////////////////////////////////////////
PT(CycleData) PipelineCyclerTrueImpl::
cycle() {
  PT(CycleData) last_val = _data[_num_stages - 1].p();
  nassertr(_lock.debug_is_locked(), last_val);
  nassertr(_dirty, last_val);

  int i;
  for (i = _num_stages - 1; i > 0; --i) {
    _data[i] = _data[i - 1];
  }

  for (i = 1; i < _num_stages; ++i) {
    if (_data[i] != _data[i - 1]) {
      // Still dirty.
      return last_val;
    }
  }

  // No longer dirty.
  _dirty = false;
  return last_val;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::set_num_stages
//       Access: Private
//  Description: Changes the number of stages in the cycler.  This is
//               only called from Pipeline::set_num_stages();
////////////////////////////////////////////////////////////////////
void PipelineCyclerTrueImpl::
set_num_stages(int num_stages) {
  nassertv(_lock.debug_is_locked());

  if (num_stages <= _num_stages) {
    // Don't bother to reallocate the array smaller; we just won't use
    // the rest of the array.
    for (int i = _num_stages; i < num_stages; ++i) {
      _data[i].clear();
    }

    _num_stages = num_stages;
    

  } else {
    // To increase the array, we must reallocate it larger.
    NPT(CycleData) *new_data = new NPT(CycleData)[num_stages];
    int i;
    for (i = 0; i < _num_stages; ++i) {
      new_data[i] = _data[i];
    }
    for (i = _num_stages; i < num_stages; ++i) {
      new_data[i] = _data[_num_stages - 1];
    }
    delete[] _data;

    _num_stages = num_stages;
    _data = new_data;
  }
}

#ifdef DEBUG_THREADS
////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::CyclerMutex::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void PipelineCyclerTrueImpl::CyclerMutex::
output(ostream &out) const {
  out << "CyclerMutex ";
  _cycler->cheat()->output(out);
}
#endif  // DEBUG_THREADS

#endif  // THREADED_PIPELINE


