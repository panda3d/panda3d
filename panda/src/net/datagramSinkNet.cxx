/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file datagramSinkNet.cxx
 * @author drose
 * @date 2009-02-15
 */

#include "pandabase.h"

#include "datagramSinkNet.h"

/**
 * Creates a new DatagramSinkNet with the indicated number of threads to
 * handle writing.  Normally num_threads should be either 0 or 1 to guarantee
 * that datagrams are delivered in the same order in which they were sent.
 */
DatagramSinkNet::
DatagramSinkNet(ConnectionManager *manager, int num_threads) :
  ConnectionWriter(manager, num_threads)
{
}

/**
 * Sends the given datagram to the target.  Returns true on success, false if
 * there is an error.  Blocks if necessary until the target is ready.
 */
bool DatagramSinkNet::
put_datagram(const Datagram &data) {
  if (_target == nullptr) {
    return false;
  }
  return send(data, _target, true);
}

/**
 * Returns true if there is an error on the target connection, or if the
 * target has never been set.
 */
bool DatagramSinkNet::
is_error() {
  return (_target == nullptr || _target->get_socket() == nullptr);
}

/**
 * Ensures that all datagrams previously written will be visible on the
 * stream.
 */
void DatagramSinkNet::
flush() {
  if (_target != nullptr) {
    _target->flush();
  }
}
