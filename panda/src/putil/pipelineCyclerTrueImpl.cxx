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
#include "reMutexHolder.h"


////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PipelineCyclerTrueImpl::
PipelineCyclerTrueImpl(CycleData *initial_data, Pipeline *pipeline) :
  _pipeline(pipeline)
{
  if (_pipeline == (Pipeline *)NULL) {
    _pipeline = Pipeline::get_render_pipeline();
  }
  _pipeline->add_cycler(this);

  _num_stages = _pipeline->get_num_stages();
  _data = new StageData[_num_stages];
  for (int i = 0; i < _num_stages; ++i) {
    _data[i]._cycle_data = initial_data;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PipelineCyclerTrueImpl::
PipelineCyclerTrueImpl(const PipelineCyclerTrueImpl &copy) :
  _pipeline(copy._pipeline)
{
  _pipeline->add_cycler(this);

  ReMutexHolder holder(copy._lock);
  _num_stages = _pipeline->get_num_stages();
  nassertv(_num_stages == copy._num_stages);
  _data = new StageData[_num_stages];
  for (int i = 0; i < _num_stages; ++i) {
    _data[i]._cycle_data = copy._data[i]._cycle_data->make_copy();
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
    _data[i]._cycle_data = copy._data[i]._cycle_data;
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

  CycleData *old_data = _data[n]._cycle_data;

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
    while (k >= 0 && _data[k]._cycle_data == old_data) {
      --k;
      --count;
    }

    if (count > 0) {
      PT(CycleData) new_data = old_data->make_copy();

      int k = n - 1;
      while (k >= 0 && _data[k]._cycle_data == old_data) {
        _data[k]._cycle_data = new_data;
        --k;
      }
      
      _data[n]._cycle_data = new_data;
    }
  }

  return _data[n]._cycle_data;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::cycle
//       Access: Private
//  Description: Cycles the data between frames.  This is only called
//               from Pipeline::cycle().
////////////////////////////////////////////////////////////////////
void PipelineCyclerTrueImpl::
cycle() {
  ReMutexHolder holder(_lock);

  for (int i = _num_stages - 1; i > 0; --i) {
    _data[i] = _data[i - 1];
  }
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
      _data[i]._cycle_data.clear();
    }

    _num_stages = num_stages;
    

  } else {
    // To increase the array, we must reallocate it larger.
    StageData *new_data = new StageData[num_stages];
    int i;
    for (i = 0; i < _num_stages; ++i) {
      new_data[i] = _data[i];
    }
    for (i = _num_stages; i < num_stages; ++i) {
      new_data[i]._cycle_data = _data[_num_stages - 1]._cycle_data;
    }
    delete[] _data;

    _num_stages = num_stages;
    _data = new_data;
  }
}

#endif  // DO_PIPELINING && HAVE_THREADS

