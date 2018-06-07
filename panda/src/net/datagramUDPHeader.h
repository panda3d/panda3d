/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file datagramUDPHeader.h
 * @author drose
 * @date 2000-02-08
 */

#ifndef DATAGRAMUDPHEADER_H
#define DATAGRAMUDPHEADER_H

#include "pandabase.h"

#include "netDatagram.h"

#include "datagramIterator.h"
#include "numeric_types.h"

static const int datagram_udp_header_size = sizeof(uint16_t);

class NetDatagram;

/**
 * A class that encapsulates the extra bytes that are sent in front of each
 * datagram to identify it when it is sent on UDP.  Like NetDatagram, this
 * class automatically handles converting its data to and from the network
 * byte ordering.
 */
class EXPCL_PANDA_NET DatagramUDPHeader {
public:
  DatagramUDPHeader(const NetDatagram &datagram);
  DatagramUDPHeader(const void *data);

  INLINE int get_datagram_checksum() const;
  INLINE std::string get_header() const;

  bool verify_datagram(const NetDatagram &datagram) const;

private:
  // The actual data for the header is stored (somewhat recursively) in its
  // own NetDatagram object.  This is just for convenience of packing and
  // unpacking the header.
  NetDatagram _header;
};

#include "datagramUDPHeader.I"

#endif
