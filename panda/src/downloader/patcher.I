// Filename: patcher.I
// Created by:  darren (20Dec00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: Patcher::get_progress
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat Patcher::
get_progress() const {
  if(_patchfile) {
    return _patchfile->get_progress();
  }

  return 0.0;
}
