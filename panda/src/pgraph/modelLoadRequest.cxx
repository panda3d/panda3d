// Filename: modelLoadRequest.cxx
// Created by:  drose (29Aug06)
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

#include "modelLoadRequest.h"
#include "loader.h"

TypeHandle ModelLoadRequest::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ModelLoadRequest::do_task
//       Access: Protected, Virtual
//  Description: Performs the task: that is, loads the one model.
////////////////////////////////////////////////////////////////////
bool ModelLoadRequest::
do_task() {
  Loader *loader;
  DCAST_INTO_R(loader, _manager, false);

  _model = loader->load_sync(_filename, _options);
  _is_ready = true;

  // Don't continue the task; we're done.
  return false;
}
