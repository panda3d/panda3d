// Filename: recentConnectionReader.cxx
// Created by:  drose (23Jun00)
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

#include "recentConnectionReader.h"
#include "config_net.h"

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
  _mutex = PR_NewLock();
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

  PR_DestroyLock(_mutex);
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
  PR_Lock(_mutex);
  if (!_available) {
    // Huh.  Nothing after all.
    PR_Unlock(_mutex);
    return false;
  }

  result = _datagram;
  _available = false;
  PR_Unlock(_mutex);
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

  PR_Lock(_mutex);
  _datagram = datagram;
  _available = true;
  PR_Unlock(_mutex);
}
