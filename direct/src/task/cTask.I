// Filename: cTask.I
// Created by:  Administrator (03Sep04)
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
//     Function: CTask::set_wake_time
//       Access: Published
//  Description: Sets the wake time of this task (for doLaters)
////////////////////////////////////////////////////////////////////
INLINE void CTask::
set_wake_time(float wake_time) {
  _wake_time = wake_time;
}

////////////////////////////////////////////////////////////////////
//     Function: CTask::get_wake_time
//       Access: Published
//  Description: Returns wake time of this task (for doLaters)
////////////////////////////////////////////////////////////////////
INLINE float CTask::
get_wake_time() const {
  return _wake_time;
}
