// Filename: cocoaGraphicsPipe.I
// Created by:  rdb (14May12)
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
//     Function: CocoaGraphicsPipe::get_display_id
//       Access: Public
//  Description: Returns the Quartz display ID associated with
//               this graphics pipe.
////////////////////////////////////////////////////////////////////
INLINE CGDirectDisplayID CocoaGraphicsPipe::
get_display_id() const {
  return _display;
}

////////////////////////////////////////////////////////////////////
//     Function: CocoaGraphicsPipe::get_nsscreen
//       Access: Public
//  Description: Returns the Cocoa NSScreen pointer associated with
//               this graphics pipe.
////////////////////////////////////////////////////////////////////
INLINE NSScreen *CocoaGraphicsPipe::
get_nsscreen() const {
  return _screen;
}
