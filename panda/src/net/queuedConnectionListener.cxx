// Filename: queuedConnectionListener.cxx
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

#include "queuedConnectionListener.h"
#include "config_net.h"

////////////////////////////////////////////////////////////////////
//     Function: QueuedConnectionListener::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
QueuedConnectionListener::
QueuedConnectionListener(ConnectionManager *manager, int num_threads) :
  ConnectionListener(manager, num_threads)
{
}

////////////////////////////////////////////////////////////////////
//     Function: QueuedConnectionListener::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
QueuedConnectionListener::
~QueuedConnectionListener() {
  // We call shutdown() here to guarantee that all threads are gone
  // before the QueuedReturn destructs.
  shutdown();
}

////////////////////////////////////////////////////////////////////
//     Function: QueuedConnectionListener::new_connection_available
//       Access: Public
//  Description: Returns true if a new connection was recently
//               established; the connection information may then be
//               retrieved via get_new_connection().
////////////////////////////////////////////////////////////////////
bool QueuedConnectionListener::
new_connection_available() {
  poll();
  return thing_available();
}

////////////////////////////////////////////////////////////////////
//     Function: QueuedConnectionListener::get_new_connection
//       Access: Public
//  Description: If a previous call to new_connection_available()
//               returned true, this function will return information
//               about the newly established connection.
//
//               The rendezvous parameter is the particular rendezvous
//               socket this new connection originally communicated
//               with; it is provided in case the ConnectionListener
//               was monitorind more than one and you care which one
//               it was.  The address parameter is the net address of
//               the new client, and new_connection is the socket of
//               the newly established connection.
//
//               The return value is true if a connection was
//               successfully returned, or false if there was, in
//               fact, no new connection.  (This may happen if there
//               are multiple threads accessing the
//               QueuedConnectionListener).
////////////////////////////////////////////////////////////////////
bool QueuedConnectionListener::
get_new_connection(PT(Connection) &rendezvous,
                   NetAddress &address,
                   PT(Connection) &new_connection) {
  ConnectionListenerData result;
  if (!get_thing(result)) {
    return false;
  }

  rendezvous = result._rendezvous;
  address = result._address;
  new_connection = result._new_connection;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: QueuedConnectionListener::get_new_connection
//       Access: Public
//  Description: This flavor of get_new_connection() simply returns a
//               new connection, assuming the user doesn't care about
//               the rendezvous socket that originated it or the
//               address it came from.
////////////////////////////////////////////////////////////////////
bool QueuedConnectionListener::
get_new_connection(PT(Connection) &new_connection) {
  PT(Connection) rendezvous;
  NetAddress address;
  return get_new_connection(rendezvous, address, new_connection);
}


////////////////////////////////////////////////////////////////////
//     Function: QueuedConnectionListener::connection_opened
//       Access: Protected, Virtual
//  Description: An internal function called by ConnectionListener()
//               when a new TCP connection has been established.  The
//               QueuedConnectionListener simply queues up this fact
//               for later retrieval by get_new_connection().
////////////////////////////////////////////////////////////////////
void QueuedConnectionListener::
connection_opened(const PT(Connection) &rendezvous,
                  const NetAddress &address,
                  const PT(Connection) &new_connection) {
  ConnectionListenerData nc;
  nc._rendezvous = rendezvous;
  nc._address = address;
  nc._new_connection = new_connection;

  if (!enqueue_thing(nc)) {
    net_cat.error()
      << "QueuedConnectionListener queue full!\n";
  }
}
