// Filename: hardwareChannel.cxx
// Created by:  mike (09Jan97)
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

#include "hardwareChannel.h"
#include "config_display.h"

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle HardwareChannel::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: HardwareChannel::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
HardwareChannel::
HardwareChannel( GraphicsWindow* window ) :
        GraphicsChannel( window ) {
  _id = 0;
  _xorg = 0;
  _yorg = 0;
  _xsize = 0;
  _ysize = 0;

  // Why do we do this?
  set_active(false);
}

////////////////////////////////////////////////////////////////////
//     Function: HardwareChannel::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
HardwareChannel::
~HardwareChannel(void) {
  return;
}

////////////////////////////////////////////////////////////////////
//     Function: HardwareChannel::window_resized
//       Access: Public, Virtual
//  Description: This is called whenever the parent window has been
//               resized; it should do whatever needs to be done to
//               adjust the channel to account for it.
////////////////////////////////////////////////////////////////////
void HardwareChannel::
window_resized(int, int) {
  // A HardwareChannel ignores window resize messages.
}
