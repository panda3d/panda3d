/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file datagramUDPHeader.cxx
 * @author drose
 * @date 2000-02-08
 */

#include "datagramUDPHeader.h"
#include "netDatagram.h"
#include "datagramIterator.h"
#include "config_net.h"

#include "pnotify.h"

/**
 * This constructor creates a header based on an already-constructed
 * NetDatagram.
 */
DatagramUDPHeader::
DatagramUDPHeader(const NetDatagram &datagram) {
  const unsigned char *begin = (const unsigned char *)datagram.get_data();
  const unsigned char *end = begin + datagram.get_length();
  uint16_t checksum = 0;
  for (const unsigned char *p = begin; p != end; ++p) {
    checksum += (uint16_t)(uint8_t)*p;
  }

  // Now pack the header.
  _header.add_uint16(checksum);
  nassertv((int)_header.get_length() == datagram_udp_header_size);
}

/**
 * This constructor decodes a header from a block of data of length
 * datagram_udp_header_size, presumably just read from a socket.
 */
DatagramUDPHeader::
DatagramUDPHeader(const void *data) : _header(data, datagram_udp_header_size) {
}

/**
 * Verifies that the indicated datagram has the appropriate length and
 * checksum.  Returns true if it matches, false otherwise.
 */
bool DatagramUDPHeader::
verify_datagram(const NetDatagram &datagram) const {
  const unsigned char *begin = (const unsigned char *)datagram.get_data();
  const unsigned char *end = begin + datagram.get_length();
  uint16_t checksum = 0;
  for (const unsigned char *p = begin; p != end; ++p) {
    checksum += (uint16_t)(uint8_t)*p;
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

    // We write the hex dump into a ostringstream first, to guarantee an
    // atomic write to the output stream in case we're threaded.

    std::ostringstream hex;
    datagram.dump_hex(hex);
    hex << "\n";
    net_cat.debug(false) << hex.str();
  }

  return false;
}
