// Filename: datagramGeneratorNet.cxx
// Created by:  drose (15Feb09)
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

#include "pandabase.h"

#include "datagramGeneratorNet.h"
#include "mutexHolder.h"
#include "lightMutexHolder.h"

////////////////////////////////////////////////////////////////////
//     Function: DatagramGeneratorNet::Constructor
//       Access: Published
//  Description: Creates a new DatagramGeneratorNet with the indicated
//               number of threads to handle requests.  Normally
//               num_threads should be either 0 or 1 to guarantee that
//               datagrams are generated in the same order in which
//               they were received.
////////////////////////////////////////////////////////////////////
DatagramGeneratorNet::
DatagramGeneratorNet(ConnectionManager *manager, int num_threads) :
  ConnectionReader(manager, num_threads),
  _dg_received(_dg_lock),
  _dg_processed(_dg_lock)
{
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramGeneratorNet::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
DatagramGeneratorNet::
~DatagramGeneratorNet() {
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramGeneratorNet::get_datagram
//       Access: Published, Virtual
//  Description: Reads the next datagram from the stream.  Blocks
//               until a datagram is available.  Returns true on
//               success, false on stream closed or error.
////////////////////////////////////////////////////////////////////
bool DatagramGeneratorNet::
get_datagram(Datagram &data) {
  if (is_polling()) {
    // Single-threaded case: we poll.  No need to lock.
    if (!thing_available()) {
      if (net_cat.is_spam()) {
        net_cat.spam()
          << "DatagramGeneratorNet polling\n";
      }
      poll();
    }
    while (!thing_available()) {
      if (is_eof()) {
        if (net_cat.is_spam()) {
          net_cat.spam()
            << "DatagramGeneratorNet returning EOF\n";
        }
        return false;
      }
      poll();
      Thread::force_yield();
    }
    bool got_dg = get_thing(data);
    nassertr(got_dg, false);

  } else {
    // Threaded case: no polling, we use mutexes and cvars to block
    // instead.
    MutexHolder holder(_dg_lock);
    while (!thing_available()) {
      if (is_eof()) {
        if (net_cat.is_spam()) {
          net_cat.spam()
            << "DatagramGeneratorNet returning EOF\n";
        }
        return false;
      }
      if (net_cat.is_spam()) {
        net_cat.spam()
          << "DatagramGeneratorNet waiting\n";
      }
      _dg_received.wait();
    }
    bool got_dg = get_thing(data);
    nassertr(got_dg, false);
    _dg_processed.notify();
  }

  if (net_cat.is_spam()) {
    net_cat.spam()
      << "DatagramGeneratorNet returning datagram of length " 
      << data.get_length() << "\n";
  }
  
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramGeneratorNet::is_eof
//       Access: Published, Virtual
//  Description: Returns true if the stream has been closed normally.
//               This test may only be made after a call to
//               get_datagram() has failed.
////////////////////////////////////////////////////////////////////
bool DatagramGeneratorNet::
is_eof() {
  // We're at eof if we have no more connected sockets.
  LightMutexHolder holder(_sockets_mutex);
  return _sockets.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramGeneratorNet::is_error
//       Access: Published, Virtual
//  Description: Returns true if the stream has an error condition.
////////////////////////////////////////////////////////////////////
bool DatagramGeneratorNet::
is_error() {
  // There's an error if any one of our connected sockets reports an error.
  LightMutexHolder holder(_sockets_mutex);
  Sockets::const_iterator si;
  for (si = _sockets.begin(); si != _sockets.end(); ++si) {
    SocketInfo *sinfo = (*si);
    if (sinfo->_error) {
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramGeneratorNet::receive_datagram
//       Access: Protected, Virtual
//  Description: An internal function called by ConnectionReader()
//               when a new datagram has become available.  This call
//               may be received in a sub-thread.
////////////////////////////////////////////////////////////////////
void DatagramGeneratorNet::
receive_datagram(const NetDatagram &datagram) {
  MutexHolder holder(_dg_lock);
  while (!enqueue_thing(datagram)) {
    _dg_processed.wait();
  }
  _dg_received.notify();
}

