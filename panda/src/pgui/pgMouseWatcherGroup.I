// Filename: pgMouseWatcherGroup.I
// Created by:  drose (09Jul01)
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
