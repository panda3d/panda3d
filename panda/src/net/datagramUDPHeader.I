// Filename: datagramUDPHeader.I
// Created by:  drose (01Aug01)
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


////////////////////////////////////////////////////////////////////
//     Function: DatagramUDPHeader::get_datagram_checksum
//       Access: Public
//  Description: Returns the checksum appropriate for the indicated
//               datagram.
////////////////////////////////////////////////////////////////////
INLINE int DatagramUDPHeader::
get_datagram_checksum() const {
  DatagramIterator di(_header);
  return di.get_uint16();
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramUDPHeader::get_header
//       Access: Public
//  Description: Returns a pointer to a block of data of length
//               datagram_udp_header_size, which can be written to the
//               network as the header information.
////////////////////////////////////////////////////////////////////
INLINE string DatagramUDPHeader::
get_header() const {
  return _header.get_message();
}
