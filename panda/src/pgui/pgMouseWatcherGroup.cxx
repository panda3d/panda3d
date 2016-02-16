/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pgMouseWatcherGroup.cxx
 * @author drose
 * @date 2001-07-09
 */

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
  // When the MouseWatcherGroup destructs for whatever reason, the
  // PGTop object should lose its MouseWatcher.
  if (_top != (PGTop *)NULL) {
    _top->_watcher_group = (PGMouseWatcherGroup *)NULL;
    _top->set_mouse_watcher((MouseWatcher *)NULL);
  }
}
