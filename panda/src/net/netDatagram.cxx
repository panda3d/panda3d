// Filename: netDatagram.cxx
// Created by:  jns (07Feb00)
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

#include "netDatagram.h"

TypeHandle NetDatagram::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: NetDatagram::Constructor
//       Access: Public
//  Description: Constructs an empty datagram.
////////////////////////////////////////////////////////////////////
NetDatagram::
NetDatagram() {
}

////////////////////////////////////////////////////////////////////
//     Function: NetDatagram::Constructor
//       Access: Public
//  Description: Constructs a datagram from an existing block of data.
////////////////////////////////////////////////////////////////////
NetDatagram::
NetDatagram(const void *data, size_t size) :
  Datagram(data, size) {
}

////////////////////////////////////////////////////////////////////
//     Function: NetDatagram::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
NetDatagram::
NetDatagram(const Datagram &copy) :
  Datagram(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: NetDatagram::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
NetDatagram::
NetDatagram(const NetDatagram &copy) :
  Datagram(copy),
  _connection(copy._connection),
  _address(copy._address)
{
}

////////////////////////////////////////////////////////////////////
//     Function: NetDatagram::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void NetDatagram::
operator = (const Datagram &copy) {
  Datagram::operator = (copy);
  _connection.clear();
  _address.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: NetDatagram::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void NetDatagram::
operator = (const NetDatagram &copy) {
  Datagram::operator = (copy);
  _connection = copy._connection;
  _address = copy._address;
}

////////////////////////////////////////////////////////////////////
//     Function: NetDatagram::clear
//       Access: Public, Virtual
//  Description: Resets the datagram to empty, in preparation for
//               building up a new datagram.
////////////////////////////////////////////////////////////////////
void NetDatagram::
clear() {
  Datagram::clear();
  _connection.clear();
  _address.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: NetDatagram::set_connection
//       Access: Public
//  Description: Specifies the socket to which the datagram should be
//               written.
////////////////////////////////////////////////////////////////////
void NetDatagram::
set_connection(const PT(Connection) &connection) {
  _connection = connection;
}

////////////////////////////////////////////////////////////////////
//     Function: NetDatagram::set_connection
//       Access: Public
//  Description: Retrieves the socket from which the datagram was
//               read, or to which it is scheduled to be written.
////////////////////////////////////////////////////////////////////
PT(Connection) NetDatagram::
get_connection() const {
  return _connection;
}

////////////////////////////////////////////////////////////////////
//     Function: NetDatagram::set_address
//       Access: Public
//  Description: Specifies the host to which the datagram should be
//               sent.
////////////////////////////////////////////////////////////////////
void NetDatagram::
set_address(const NetAddress &address) {
  _address = address;
}

////////////////////////////////////////////////////////////////////
//     Function: NetDatagram::set_address
//       Access: Public
//  Description: Retrieves the host from which the datagram was
//               read, or to which it is scheduled to be sent.
////////////////////////////////////////////////////////////////////
const NetAddress &NetDatagram::
get_address() const {
  return _address;
}
