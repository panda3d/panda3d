// Filename: pipelineCyclerTrueImpl.cxx
// Created by:  drose (31Jan06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "pipelineCyclerTrueImpl.h"

#if defined(DO_PIPELINING) && defined(HAVE_THREADS)

#include "config_util.h"
#include "pipeline.h"

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PipelineCyclerTrueImpl::
PipelineCyclerTrueImpl(CycleData *initial_data, Pipeline *pipeline) :
  _pipeline(pipeline),
  _dirty(false)
{
  if (_pipeline == (Pipeline *)NULL) {
    _pipeline = Pipeline::get_render_pipeline();
  }
  _pipeline->add_cycler(this);

  _num_stages = _pipeline->get_num_stages();
  _data = new PT(CycleData)[_num_stages];
  for (int i = 0; i < _num_stages; ++i) {
    _data[i] = initial_data;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PipelineCyclerTrueImpl::
PipelineCyclerTrueImpl(const PipelineCyclerTrueImpl &copy) :
  _pipeline(copy._pipeline),
  _dirty(false)
{
  ReMutexHolder holder(_lock);
  _pipeline->add_cycler(this);
  if (copy._dirty) {
    _pipeline->add_dirty_cycler(this);
  }

  ReMutexHolder holder2(copy._lock);
  _num_stages = _pipeline->get_num_stages();
  nassertv(_num_stages == copy._num_stages);
  _data = new PT(CycleData)[_num_stages];

  // It's important that we preserve pointerwise equivalence in the
  // copy: if a and b of the original pipeline are the same pointer,
  // then a' and b' of the copied pipeline should be the same pointer
  // (but a' must be a different pointer than a).  This is important
  // because we rely on pointer equivalence to determine whether an
  // adjustment at a later stage in the pipeline is automatically
  // propagated backwards.
  typedef pmap<CycleData *, PT(CycleData) > Pointers;
  Pointers pointers;

  for (int i = 0; i < _num_stages; ++i) {
    PT(CycleData) &new_pt = pointers[copy._data[i]];
    if (new_pt == NULL) {
      new_pt = copy._data[i]->make_copy();
    }
    _data[i] = new_pt;
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

  nassertv(_num_stages == copy._num_stages);
  for (int i = 0; i < _num_stages; ++i) {
    _data[i] = copy._data[i];
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
  delete[] _data;
  _data = NULL;
  _num_stages = 0;

  _pipeline->remove_cycler(this);
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
write_stage(int n) {
  _lock.lock();

#ifndef NDEBUG
  nassertd(n >= 0 && n < _num_stages) {
    _lock.release();
    return NULL;
  }
#endif  // NDEBUG

  CycleData *old_data = _data[n];

  if (old_data->get_ref_count() != 1) {
    // Copy-on-write.

    // There's a special problem that happens when we write to a stage
    // other than stage 0.  If we do this, when the next frame cycles,
    // the changes that we record to stage n will be lost when the
    // data from stage (n - 1) is cycled into place.  This can be
    // wasteful, especially if we are updating a cached value (which
    // is generally the case when we are writing to stages other than
    // stage 0).

    // To minimize this, we make a special exception: whenever we
    // write to stage n, if stage (n - 1) has the same pointer, we
    // will write to stage (n - 1) at the same time, and so on all the
    // way back to stage 0 or the last different stage.

    // On the other hand, if *all* of the instances of this pointer
    // are found in stages k .. n, then we don't need to do anything
    // at all.
    int count = old_data->get_ref_count() - 1;
    int k = n - 1;
    while (k >= 0 && _data[k] == old_data) {
      --k;
      --count;
    }

    if (count > 0) {
      PT(CycleData) new_data = old_data->make_copy();

      int k = n - 1;
      while (k >= 0 && _data[k] == old_data) {
        _data[k] = new_data;
        --k;
      }
      
      _data[n] = new_data;

      // Now we have differences between some of the data pointers, so
      // we're "dirty".  Mark it so.
      if (!_dirty) {
        _pipeline->add_dirty_cycler(this);
      }
    }
  }

  return _data[n];
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
  PT(CycleData) last_val = _data[_num_stages - 1];
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
    PT(CycleData) *new_data = new PT(CycleData)[num_stages];
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

#endif  // DO_PIPELINING && HAVE_THREADS

