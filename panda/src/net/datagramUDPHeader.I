// Filename: datagramUDPHeader.I
// Created by:  drose (01Aug01)
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
