// Filename: cIntervalManager.I
// Created by:  drose (10Sep02)
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
//     Function: CIntervalManager::set_event_queue
//       Access: Published
//  Description: Specifies a custom event queue to be used for
//               throwing done events from intervals as they finish.
//               If this is not specified, the global event queue is
//               used.
//
//               The caller maintains ownership of the EventQueue
//               object; it is the caller's responsibility to ensure
//               that the supplied EventQueue does not destruct during
//               the lifetime of the CIntervalManager.
////////////////////////////////////////////////////////////////////
INLINE void CIntervalManager::
set_event_queue(EventQueue *event_queue) {
  _event_queue = event_queue;
}

////////////////////////////////////////////////////////////////////
//     Function: CIntervalManager::get_event_queue
//       Access: Published
//  Description: Returns the custom event queue to be used for
//               throwing done events from intervals as they finish.
////////////////////////////////////////////////////////////////////
INLINE EventQueue *CIntervalManager::
get_event_queue() const {
  return _event_queue;
}

INLINE ostream &
operator << (ostream &out, const CIntervalManager &ival_mgr) {
  ival_mgr.output(out);
  return out;
}

