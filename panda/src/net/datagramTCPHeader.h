/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file datagramTCPHeader.h
 * @author drose
 * @date 2000-02-08
 */

#ifndef DATAGRAMTCPHEADER_H
#define DATAGRAMTCPHEADER_H

#include "pandabase.h"

#include "netDatagram.h"

#include "datagramIterator.h"
#include "numeric_types.h"

static const int datagram_tcp16_header_size = sizeof(uint16_t);
static const int datagram_tcp32_header_size = sizeof(uint32_t);

class NetDatagram;

/**
 * A class that encapsulates the extra bytes that are sent in front of each
 * datagram to identify it when it is sent on TCP.  This is similar to
 * DatagramUDPHeader, except it does not include a checksum, since this is
 * unnecessary on UDP.
 */
class EXPCL_PANDA_NET DatagramTCPHeader {
public:
  DatagramTCPHeader(const NetDatagram &datagram, int header_size);
  DatagramTCPHeader(const void *data, int header_size);

  int get_datagram_size(int header_size) const;
  INLINE std::string get_header() const;

  bool verify_datagram(const NetDatagram &datagram, int header_size) const;

private:
  // The actual data for the header is stored (somewhat recursively) in its
  // own NetDatagram object.  This is just for convenience of packing and
  // unpacking the header.
  NetDatagram _header;
};

#include "datagramTCPHeader.I"

#endif
