// Filename: datagramUDPHeader.cxx
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

#include "datagramUDPHeader.h"
#include "netDatagram.h"
#include "datagramIterator.h"
#include "config_net.h"

#include "notify.h"

////////////////////////////////////////////////////////////////////
//     Function: DatagramUDPHeader::Constructor
//       Access: Public
//  Description: This constructor creates a header based on an
//               already-constructed NetDatagram.
////////////////////////////////////////////////////////////////////
DatagramUDPHeader::
DatagramUDPHeader(const NetDatagram &datagram) {
  const string &str = datagram.get_message();
  PRUint16 checksum = 0;
  for (size_t p = 0; p < str.size(); p++) {
    checksum += (PRUint16)(PRUint8)str[p];
  }

  // Now pack the header.
  _header.add_uint16(checksum);
  nassertv((int)_header.get_length() == datagram_udp_header_size);
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramUDPHeader::Constructor
//       Access: Public
//  Description: This constructor decodes a header from a block of
//               data of length datagram_udp_header_size, presumably
//               just read from a socket.
////////////////////////////////////////////////////////////////////
DatagramUDPHeader::
DatagramUDPHeader(const void *data) : _header(data, datagram_udp_header_size) {
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramUDPHeader::verify_datagram
//       Access: Public
//  Description: Verifies that the indicated datagram has the
//               appropriate length and checksum.  Returns true if it
//               matches, false otherwise.
////////////////////////////////////////////////////////////////////
bool DatagramUDPHeader::
verify_datagram(const NetDatagram &datagram) const {
  const string &str = datagram.get_message();

  PRUint16 checksum = 0;
  for (size_t p = 0; p < str.size(); p++) {
    checksum += (PRUint16)(PRUint8)str[p];
  }

  if (checksum == get_datagram_checksum()) {
    return true;
  }

  if (net_cat.is_debug()) {
    net_cat.debug()
      << "Invalid datagram!\n";
    if (checksum != get_datagram_checksum()) {
      net_cat.debug()
        << "  checksum is " << checksum << ", header reports "
        << get_datagram_checksum() << "\n";
    }

    // We write the hex dump into a ostringstream first, to guarantee
    // an atomic write to the output stream in case we're threaded.

    ostringstream hex;
    datagram.dump_hex(hex);
    hex << "\n";
    net_cat.debug(false) << hex.str();
  }

  return false;
}
