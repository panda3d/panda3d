// Filename: queuedConnectionReader.cxx
// Created by:  drose (08Feb00)
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

#include "queuedConnectionReader.h"
#include "config_net.h"

////////////////////////////////////////////////////////////////////
//     Function: QueuedConnectionReader::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
QueuedConnectionReader::
QueuedConnectionReader(ConnectionManager *manager, int num_threads) :
  ConnectionReader(manager, num_threads)
{
}

////////////////////////////////////////////////////////////////////
//     Function: QueuedConnectionReader::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
QueuedConnectionReader::
~QueuedConnectionReader() {
  // We call shutdown() here to guarantee that all threads are gone
  // before the QueuedReturn destructs.
  shutdown();
}

////////////////////////////////////////////////////////////////////
//     Function: QueuedConnectionReader::data_available
//       Access: Public
//  Description: Returns true if a datagram is available on the queue;
//               call get_data() to extract the datagram.
////////////////////////////////////////////////////////////////////
bool QueuedConnectionReader::
data_available() {
  poll();
  return thing_available();
}

////////////////////////////////////////////////////////////////////
//     Function: QueuedConnectionReader::get_data
//       Access: Public
//  Description: If a previous call to data_available() returned
//               true, this function will return the datagram that has
//               become available.
//
//               The return value is true if a datagram was
//               successfully returned, or false if there was, in
//               fact, no datagram available.  (This may happen if
//               there are multiple threads accessing the
//               QueuedConnectionReader).
////////////////////////////////////////////////////////////////////
bool QueuedConnectionReader::
get_data(NetDatagram &result) {
  return get_thing(result);
}

////////////////////////////////////////////////////////////////////
//     Function: QueuedConnectionReader::get_data
//       Access: Public
//  Description: This flavor of QueuedConnectionReader::get_data(),
//               works like the other, except that it only fills a
//               Datagram object, not a NetDatagram object.  This
//               means that the Datagram cannot be queried for its
//               source Connection and/or NetAddress, but it is useful
//               in all other respects.
////////////////////////////////////////////////////////////////////
bool QueuedConnectionReader::
get_data(Datagram &result) {
  NetDatagram nd;
  if (!get_thing(nd)) {
    return false;
  }
  result = nd;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: QueuedConnectionReader::receive_datagram
//       Access: Protected, Virtual
//  Description: An internal function called by ConnectionReader()
//               when a new datagram has become available.  The
//               QueuedConnectionReader simply queues it up for later
//               retrieval by get_data().
////////////////////////////////////////////////////////////////////
void QueuedConnectionReader::
receive_datagram(const NetDatagram &datagram) {
  if (net_cat.is_debug()) {
    net_cat.debug()
      << "Received datagram of " << datagram.get_length()
      << " bytes\n";
  }

  if (!enqueue_thing(datagram)) {
    net_cat.error()
      << "QueuedConnectionReader queue full!\n";
  }
}
