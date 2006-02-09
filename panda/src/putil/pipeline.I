// Filename: pipeline.I
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


////////////////////////////////////////////////////////////////////
//     Function: Pipeline::get_render_pipeline
//       Access: Public, Static
//  Description: Returns a pointer to the global render pipeline.
////////////////////////////////////////////////////////////////////
INLINE Pipeline *Pipeline::
get_render_pipeline() {
  if (_render_pipeline == (Pipeline *)NULL) {
    make_render_pipeline();
  }
  return _render_pipeline;
}

////////////////////////////////////////////////////////////////////
//     Function: Pipeline::set_min_stages
//       Access: Public
//  Description: Ensures that at least the indicated number of stages
//               are in the pipeline.
////////////////////////////////////////////////////////////////////
INLINE void Pipeline::
set_min_stages(int min_stages) {
  set_num_stages(max(min_stages, get_num_stages()));
}

////////////////////////////////////////////////////////////////////
//     Function: Pipeline::get_num_stages
//       Access: Public
//  Description: Returns the number of stages required for the
//               pipeline.
////////////////////////////////////////////////////////////////////
INLINE int Pipeline::
get_num_stages() const {
  return _num_stages;
}
