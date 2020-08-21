/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file datagramTCPHeader.cxx
 * @author drose
 * @date 2000-02-08
 */

#include "datagramTCPHeader.h"
#include "netDatagram.h"
#include "datagramIterator.h"
#include "config_net.h"

#include "pnotify.h"

/**
 * This constructor creates a header based on an already-constructed
 * NetDatagram.
 */
DatagramTCPHeader::
DatagramTCPHeader(const NetDatagram &datagram, int header_size) {
  size_t length = datagram.get_length();
  switch (header_size) {
  case 0:
    break;

  case datagram_tcp16_header_size:
    {
      uint16_t size = (uint16_t)length;
      nassertv((size_t)size == length);
      _header.add_uint16(size);
    }
    break;

  case datagram_tcp32_header_size:
    {
      uint32_t size = (uint32_t)length;
      nassertv((size_t)size == length);
      _header.add_uint32(size);
    }
    break;

  default:
    nassert_raise("invalid header size");
    return;
  }

  nassertv((int)_header.get_length() == header_size);
}

/**
 * This constructor decodes a header from a block of data of length
 * datagram_tcp_header_size, presumably just read from a socket.
 */
DatagramTCPHeader::
DatagramTCPHeader(const void *data, int header_size) :
  _header(data, header_size)
{
}

/**
 * Returns the number of bytes in the associated datagram.
 */
int DatagramTCPHeader::
get_datagram_size(int header_size) const {
  DatagramIterator di(_header);
  switch (header_size) {
  case 0:
    return 0;

  case datagram_tcp16_header_size:
    return di.get_uint16();

  case datagram_tcp32_header_size:
    return di.get_uint32();
  }

  return -1;
}

/**
 * Verifies that the indicated datagram has the appropriate length.  Returns
 * true if it matches, false otherwise.
 */
bool DatagramTCPHeader::
verify_datagram(const NetDatagram &datagram, int header_size) const {
  if (header_size == 0) {
    // No way to validate without a header, so everything is valid.
    return true;
  }

  int actual_size = (int)datagram.get_length();
  int expected_size = get_datagram_size(header_size);
  if (actual_size == expected_size) {
    return true;
  }

  if (net_cat.is_debug()) {
    net_cat.debug()
      << "Invalid datagram!  Size is " << actual_size
      << " bytes, header reports " << expected_size << "\n";

    // We write the hex dump into a ostringstream first, to guarantee an
    // atomic write to the output stream in case we're threaded.

    std::ostringstream hex;
    datagram.dump_hex(hex);
    hex << "\n";
    net_cat.debug() << hex.str();
  }

  return false;
}
