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


////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerBase::Constructor
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

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerBase::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PipelineCyclerBase::
~PipelineCyclerBase() {
  nassertv(_read_count == 0 && _write_count == 0 && _stage_count == 0);
}

