// Filename: pipeline.cxx
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

#include "pipeline.h"

Pipeline *Pipeline::_render_pipeline = (Pipeline *)NULL;

////////////////////////////////////////////////////////////////////
//     Function: Pipeline::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
Pipeline::
Pipeline(const string &name) :
  Namable(name)
{
}

////////////////////////////////////////////////////////////////////
//     Function: Pipeline::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
Pipeline::
~Pipeline() {
}

////////////////////////////////////////////////////////////////////
//     Function: Pipeline::cycle
//       Access: Public
//  Description: Flows all the pipeline data down to the next stage.
////////////////////////////////////////////////////////////////////
void Pipeline::
cycle() {
  pre_cycle();
}

////////////////////////////////////////////////////////////////////
//     Function: Pipeline::pre_cycle
//       Access: Protected, Virtual
//  Description: A callback function intended to be overridden by a
//               derived class to perform whatever operations should
//               be done before cycling the pipeline.
////////////////////////////////////////////////////////////////////
void Pipeline::
pre_cycle() {
}

////////////////////////////////////////////////////////////////////
//     Function: Pipeline::make_render_pipeline
//       Access: Private, Static
//  Description: 
////////////////////////////////////////////////////////////////////
void Pipeline::
make_render_pipeline() {
  nassertv(_render_pipeline == (Pipeline *)NULL);
  _render_pipeline = new Pipeline("render");
}

