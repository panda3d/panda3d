// Filename: datagramTCPHeader.h
// Created by:  drose (08Feb00)
//
////////////////////////////////////////////////////////////////////

#ifndef DATAGRAMTCPHEADER_H
#define DATAGRAMTCPHEADER_H

#include <pandabase.h>

#include "netDatagram.h"

#include <prtypes.h>

static const int datagram_tcp_header_size = sizeof(PRUint16);

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
  DatagramTCPHeader(const NetDatagram &datagram);
  DatagramTCPHeader(const void *data);

  int get_datagram_size() const;

  const string &get_header() const;

  bool verify_datagram(const NetDatagram &datagram) const;

private:
  // The actual data for the header is stored (somewhat recursively)
  // in its own NetDatagram object.  This is just for convenience of
  // packing and unpacking the header.
  NetDatagram _header;
};

#endif

 
