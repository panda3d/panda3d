// Filename: graphicsChannel.I
// Created by:  frang (07Mar99)
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


////////////////////////////////////////////////////////////////////
//     Function: GraphicsChannel::set_active
//       Access: Public
//  Description: Sets the active flag on the channel.  If the channel
//               is marked as inactive, nothing will be rendered.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsChannel::
set_active(bool active) {
  if (active != _is_active) {
    _is_active = active;
    win_display_regions_changed();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsChannel::is_active
//       Access: Public
//  Description: Returns the active flag on the channel.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsChannel::
is_active() const {
  return _is_active;
}

