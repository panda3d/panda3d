// Filename: pipelineCyclerBase.cxx
// Created by:  drose (21Feb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "pipelineCyclerBase.h"


#ifdef DO_PIPELINING
// The following implementations are to support compiled-in pipeline
// sanity checks.

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerBase::Constructor (sanity-check)
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PipelineCyclerBase::
PipelineCyclerBase(CycleData *initial_data, Pipeline *pipeline) :
  _data(initial_data),
  _pipeline(pipeline),
  _read_count(0),
  _write_count(0),
  _stage_count(0)
{
  if (_pipeline == (Pipeline *)NULL) {
    _pipeline = Pipeline::get_render_pipeline();
  }
}


#else  // !DO_PIPELINING
// The following implementations are provided for when pipelining is
// not compiled in.  They are trivial functions that do as little as
// possible.

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerBase::Constructor (trivial)
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PipelineCyclerBase::
PipelineCyclerBase(CycleData *initial_data, Pipeline *) {
  // In the trivial implementation, a derived class (the
  // PipelineCycler template class) stores the CycleData object
  // directly within itself, and since we have no data members or
  // virtual functions, we get away with assuming the pointer is the
  // same as the 'this' pointer.

  // If this turns out not to be true on a particular platform, we
  // will have to store the pointer in this class, for a little bit of
  // extra overhead.
  nassertv(initial_data == (CycleData *)this);
}

#endif
