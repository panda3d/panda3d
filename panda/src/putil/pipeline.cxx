// Filename: pipeline.cxx
// Created by:  drose (21Feb02)
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
//       Access: Public, Virtual
//  Description: Flows all the pipeline data down to the next stage.
////////////////////////////////////////////////////////////////////
void Pipeline::
cycle() {
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

