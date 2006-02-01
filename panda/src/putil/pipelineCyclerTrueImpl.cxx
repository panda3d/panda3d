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

#include "pipeline.h"


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
    _data[i]._cycle_data = initial_data->make_copy();
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
  nassertv(_num_stages == copy._num_stages);
  for (int i = 0; i < _num_stages; ++i) {
    _data[i]._cycle_data = copy._data[i]._cycle_data->make_copy();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PipelineCyclerTrueImpl::
~PipelineCyclerTrueImpl() {
  _pipeline->remove_cycler(this);

  delete[] _data;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::cycle
//       Access: Private
//  Description: Cycles the data between frames.  This is only called
//               from Pipeline::cycle().
////////////////////////////////////////////////////////////////////
void PipelineCyclerTrueImpl::
cycle() {
  for (int i = _num_stages - 1; i > 0; --i) {
    _data[i] = _data[i - 1];
  }
  _data[0]._cycle_data = _data[0]._cycle_data->make_copy();
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrueImpl::set_num_stages
//       Access: Private
//  Description: Changes the number of stages in the cycler.  This is
//               only called from Pipeline::set_num_stages();
////////////////////////////////////////////////////////////////////
void PipelineCyclerTrueImpl::
set_num_stages(int num_stages) {
  if (num_stages <= _num_stages) {
    // Don't bother to reallocate the array.
    for (int i = _num_stages; i < num_stages; ++i) {
      _data[i]._cycle_data.clear();
    }

    _num_stages = num_stages;
    

  } else {
    // Increase the array.
    StageData *new_data = new StageData[num_stages];
    int i;
    for (i = 0; i < _num_stages; ++i) {
      new_data[i] = _data[i];
    }
    for (i = _num_stages; i < num_stages; ++i) {
      new_data[i]._cycle_data = _data[_num_stages - 1]._cycle_data->make_copy();
    }
    delete[] _data;

    _num_stages = num_stages;
    _data = new_data;
  }
}

#endif  // DO_PIPELINING && HAVE_THREADS

