// Filename: pgMouseWatcherGroup.cxx
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

#include "pgMouseWatcherGroup.h"
#include "pgTop.h"

TypeHandle PGMouseWatcherGroup::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PGMouseWatcherGroup::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
PGMouseWatcherGroup::
~PGMouseWatcherGroup() {
  cerr << "~PGMouseWatcherGroup()\n";
  // When the MouseWatcherGroup destructs for whatever reason, the
  // PGTop object should lose its MouseWatcher.
  if (_top != (PGTop *)NULL) {
    _top->_watcher_group = (PGMouseWatcherGroup *)NULL;
    _top->set_mouse_watcher((MouseWatcher *)NULL);
  }
}
