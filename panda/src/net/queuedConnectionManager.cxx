// Filename: queuedConnectionManager.cxx
// Created by:  drose (09Feb00)
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

#include "queuedConnectionManager.h"

#include <algorithm>

////////////////////////////////////////////////////////////////////
//     Function: QueuedConnectionManager::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
QueuedConnectionManager::
QueuedConnectionManager() {
}

////////////////////////////////////////////////////////////////////
//     Function: QueuedConnectionManager::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
QueuedConnectionManager::
~QueuedConnectionManager() {
}

////////////////////////////////////////////////////////////////////
//     Function: QueuedConnectionManager::reset_connection_available
//       Access: Public
//  Description: Returns true if one of the readers/writers/listeners
//               reported a connection reset recently.  If so, the
//               particular connection that has been reset can be
//               extracted via get_reset_connection().
//
//               Only connections which were externally reset are
//               certain to appear in this list.  Those which were
//               explicitly closed via a call to close_connection()
//               may or may not be reported.  Furthermore, it is the
//               responsibility of the caller to subsequently call
//               close_connection() with any connection reported reset
//               by this call.  (There is no harm in calling
//               close_connection() more than once on a given socket.)
////////////////////////////////////////////////////////////////////
bool QueuedConnectionManager::
reset_connection_available() const {
  return thing_available();
}

////////////////////////////////////////////////////////////////////
//     Function: QueuedConnectionManager::get_reset_connection
//       Access: Public
//  Description: If a previous call to reset_connection_available()
//               returned true, this function will return information
//               about the newly reset connection.
//
//               Only connections which were externally reset are
//               certain to appear in this list.  Those which were
//               explicitly closed via a call to close_connection()
//               may or may not be reported.  Furthermore, it is the
//               responsibility of the caller to subsequently call
//               close_connection() with any connection reported reset
//               by this call.  (There is no harm in calling
//               close_connection() more than once on a given socket.)
//
//               The return value is true if a connection was
//               successfully returned, or false if there was, in
//               fact, no reset connection.  (This may happen if
//               there are multiple threads accessing the
//               QueuedConnectionManager).
////////////////////////////////////////////////////////////////////
bool QueuedConnectionManager::
get_reset_connection(PT(Connection) &connection) {
  return get_thing(connection);
}


////////////////////////////////////////////////////////////////////
//     Function: QueuedConnectionManager::connection_reset
//       Access: Protected, Virtual
//  Description: An internal function called by the ConnectionReader,
//               ConnectionWriter, or ConnectionListener when a
//               connection has been externally reset.  This adds the
//               connection to the queue of those which have recently
//               been reset.
////////////////////////////////////////////////////////////////////
void QueuedConnectionManager::
connection_reset(const PT(Connection) &connection, bool okflag) {
  ConnectionManager::connection_reset(connection, okflag);

  // Largely, we don't care if this particular queue fills up.  If it
  // does, it probably just means the user isn't bothering to track
  // this.
  enqueue_unique_thing(connection);
}
