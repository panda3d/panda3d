/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file netDatagram.cxx
 * @author jns
 * @date 2000-02-07
 */

#include "netDatagram.h"

TypeHandle NetDatagram::_type_handle;

/**
 * Constructs an empty datagram.
 */
NetDatagram::
NetDatagram() {
}

/**
 * Constructs a datagram from an existing block of data.
 */
NetDatagram::
NetDatagram(const void *data, size_t size) :
  Datagram(data, size) {
}

/**
 *
 */
NetDatagram::
NetDatagram(const Datagram &copy) :
  Datagram(copy)
{
}

/**
 *
 */
NetDatagram::
NetDatagram(const NetDatagram &copy) :
  Datagram(copy),
  _connection(copy._connection),
  _address(copy._address)
{
}

/**
 *
 */
void NetDatagram::
operator = (const Datagram &copy) {
  Datagram::operator = (copy);
  _connection.clear();
  _address.clear();
}

/**
 *
 */
void NetDatagram::
operator = (const NetDatagram &copy) {
  Datagram::operator = (copy);
  _connection = copy._connection;
  _address = copy._address;
}

/**
 * Resets the datagram to empty, in preparation for building up a new
 * datagram.
 */
void NetDatagram::
clear() {
  Datagram::clear();
  _connection.clear();
  _address.clear();
}

/**
 * Specifies the socket to which the datagram should be written.
 */
void NetDatagram::
set_connection(const PT(Connection) &connection) {
  _connection = connection;
}

/**
 * Retrieves the socket from which the datagram was read, or to which it is
 * scheduled to be written.
 */
PT(Connection) NetDatagram::
get_connection() const {
  return _connection;
}

/**
 * Specifies the host to which the datagram should be sent.
 */
void NetDatagram::
set_address(const NetAddress &address) {
  _address = address;
}

/**
 * Retrieves the host from which the datagram was read, or to which it is
 * scheduled to be sent.
 */
const NetAddress &NetDatagram::
get_address() const {
  return _address;
}
