// Filename: glxGraphicsWindow.I
// Created by:  mike (07Mar99)
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
//     Function: add_event_mask
//       Access: Protected
//  Description: Ensures we only update window events once
////////////////////////////////////////////////////////////////////
INLINE void glxGraphicsWindow::add_event_mask(long event_mask)
{
  if ((_event_mask & event_mask) != event_mask) {
    _event_mask |= event_mask;
    _change_mask |= GLXWIN_EVENT;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: remove_event_mask
//       Access: Protected
//  Description: Ensures we only update window events once
////////////////////////////////////////////////////////////////////
INLINE void glxGraphicsWindow::remove_event_mask(long event_mask)
{
  if (_event_mask & event_mask) {
    _event_mask &= ~event_mask;
    _change_mask |= GLXWIN_EVENT;
  }
}
