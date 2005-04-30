// Filename: datagramTCPHeader.h
// Created by:  drose (08Feb00)
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

#ifndef DATAGRAMTCPHEADER_H
#define DATAGRAMTCPHEADER_H

#include "pandabase.h"

#include "netDatagram.h"

#include "datagramIterator.h"

#include <prtypes.h>

static const int datagram_tcp16_header_size = sizeof(PRUint16);
static const int datagram_tcp32_header_size = sizeof(PRUint32);

class NetDatagram;

////////////////////////////////////////////////////////////////////
//       Class : DatagramTCPHeader
// Description : A class that encapsulates the extra bytes that are
//               sent in front of each datagram to identify it when it
//               is sent on TCP.  This is similar to
//               DatagramUDPHeader, except it does not include a
//               checksum, since this is unnecessary on UDP.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA DatagramTCPHeader {
public:
  DatagramTCPHeader(const NetDatagram &datagram, int header_size);
  DatagramTCPHeader(const void *data, int header_size);

  int get_datagram_size(int header_size) const;
  INLINE string get_header() const;

  bool verify_datagram(const NetDatagram &datagram, int header_size) const;

private:
  // The actual data for the header is stored (somewhat recursively)
  // in its own NetDatagram object.  This is just for convenience of
  // packing and unpacking the header.
  NetDatagram _header;
};

#include "datagramTCPHeader.I"

#endif


