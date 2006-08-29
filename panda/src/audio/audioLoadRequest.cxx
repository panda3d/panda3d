// Filename: audioLoadRequest.cxx
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

#include "audioLoadRequest.h"
#include "audioManager.h"

TypeHandle AudioLoadRequest::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: AudioLoadRequest::do_task
//       Access: Protected, Virtual
//  Description: Performs the task: that is, loads the one sound file.
////////////////////////////////////////////////////////////////////
bool AudioLoadRequest::
do_task() {
  _sound = _audio_manager->get_sound(_filename, _positional);
  _is_ready = true;

  // Don't continue the task; we're done.
  return false;
}
