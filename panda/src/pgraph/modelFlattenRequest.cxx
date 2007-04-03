// Filename: modelFlattenRequest.cxx
// Created by:  drose (30Mar07)
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

#include "modelFlattenRequest.h"
#include "nodePath.h"

TypeHandle ModelFlattenRequest::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ModelFlattenRequest::do_task
//       Access: Protected, Virtual
//  Description: Performs the task: that is, copies and flattens the
//               model.
////////////////////////////////////////////////////////////////////
bool ModelFlattenRequest::
do_task() {
  // We make another instance of the original node, so we can safely
  // flatten that without affecting the original copy.
  NodePath np("flatten_root");
  np.attach_new_node(_orig);
  np.flatten_strong();
  _model = np.get_child(0).node();
  _is_ready = true;

  // Don't continue the task; we're done.
  return false;
}
