// Filename: pgMouseWatcherGroup.I
// Created by:  drose (09Jul01)
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
//     Function: PGMouseWatcherGroup::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PGMouseWatcherGroup::
PGMouseWatcherGroup(qpPGTop *top) : _qptop(top) {
}

////////////////////////////////////////////////////////////////////
//     Function: PGMouseWatcherGroup::clear_top
//       Access: Public
//  Description: Called by the PGTop object to indicate that it is no
//               longer keeping the pointer to the PGMouseWatcherGroup
//               object.
////////////////////////////////////////////////////////////////////
INLINE void PGMouseWatcherGroup::
clear_top(qpPGTop *top) {
  nassertv(_qptop == top);
  _qptop = (qpPGTop *)NULL;
}
