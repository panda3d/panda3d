// Filename: mouseInterfaceNode.I
// Created by:  drose (11Jun04)
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
//     Function: MouseInterfaceNode::is_down
//       Access: Protected
//  Description: Returns true if the indicated button (which must have
//               been specified in a previous call to watch_button())
//               is known to be held down, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool MouseInterfaceNode::
is_down(ButtonHandle button) const {
  return _current_button_state.is_down(button);
}
