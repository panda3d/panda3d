// Filename: recentConnectionReader.cxx
// Created by:  drose (23Jun00)
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

#include "recentConnectionReader.h"
#include "config_net.h"
#include "lightMutexHolder.h"

////////////////////////////////////////////////////////////////////
//     Function: RecentConnectionReader::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
RecentConnectionReader::
RecentConnectionReader(ConnectionManager *manager) :
  ConnectionReader(manager, 1)
{
  // We should not receive any datagrams before the constructor is
  // done initializing, or our thread may get confused.  Fortunately
  // this should be impossible, because we can't receive datagrams
  // before we call add_connection().
  _available = false;
}

////////////////////////////////////////////////////////////////////
//     Function: RecentConnectionReader::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
RecentConnectionReader::
~RecentConnectionReader() {
  // We call shutdown() here to guarantee that all threads are gone
  // before the RecentConnectionReader destructs.
  shutdown();
}

////////////////////////////////////////////////////////////////////
//     Function: RecentConnectionReader::data_available
//       Access: Public
//  Description: Returns true if a datagram is available on the queue;
//               call get_data() to extract the datagram.
////////////////////////////////////////////////////////////////////
bool RecentConnectionReader::
data_available() {
  return _available;
}

////////////////////////////////////////////////////////////////////
//     Function: RecentConnectionReader::get_data
//       Access: Public
//  Description: If a previous call to data_available() returned
//               true, this function will return the datagram that has
//               become available.
//
//               The return value is true if a datagram was
//               successfully returned, or false if there was, in
//               fact, no datagram available.  (This may happen if
//               there are multiple threads accessing the
//               RecentConnectionReader).
////////////////////////////////////////////////////////////////////
bool RecentConnectionReader::
get_data(NetDatagram &result) {
  LightMutexHolder holder(_mutex);
  if (!_available) {
    // Huh.  Nothing after all.
    return false;
  }

  result = _datagram;
  _available = false;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: RecentConnectionReader::get_data
//       Access: Public
//  Description: This flavor of RecentConnectionReader::get_data(),
//               works like the other, except that it only fills a
//               Datagram object, not a NetDatagram object.  This
//               means that the Datagram cannot be queried for its
//               source Connection and/or NetAddress, but it is useful
//               in all other respects.
////////////////////////////////////////////////////////////////////
bool RecentConnectionReader::
get_data(Datagram &result) {
  NetDatagram nd;
  if (!get_data(nd)) {
    return false;
  }
  result = nd;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: RecentConnectionReader::receive_datagram
//       Access: Protected, Virtual
//  Description: An internal function called by ConnectionReader()
//               when a new datagram has become available.  The
//               RecentConnectionReader simply queues it up for later
//               retrieval by get_data().
////////////////////////////////////////////////////////////////////
void RecentConnectionReader::
receive_datagram(const NetDatagram &datagram) {
  if (net_cat.is_debug()) {
    net_cat.debug()
      << "Received datagram of " << datagram.get_length()
      << " bytes\n";
  }

  LightMutexHolder holder(_mutex);
  _datagram = datagram;
  _available = true;
}
