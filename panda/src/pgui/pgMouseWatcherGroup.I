// Filename: pgMouseWatcherGroup.I
// Created by:  drose (09Jul01)
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
//     Function: PGMouseWatcherGroup::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PGMouseWatcherGroup::
PGMouseWatcherGroup(PGTop *top) : _top(top) {
}

////////////////////////////////////////////////////////////////////
//     Function: PGMouseWatcherGroup::clear_top
//       Access: Public
//  Description: Called by the PGTop object to indicate that it is no
//               longer keeping the pointer to the PGMouseWatcherGroup
//               object.
////////////////////////////////////////////////////////////////////
INLINE void PGMouseWatcherGroup::
clear_top(PGTop *top) {
  nassertv(_top == top);
  _top = (PGTop *)NULL;
}
