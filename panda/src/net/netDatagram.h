/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file netDatagram.h
 * @author jns
 * @date 2000-02-07
 */

#ifndef NETDATAGRAM_H
#define NETDATAGRAM_H

#include "pandabase.h"

#include "connection.h"
#include "netAddress.h"

#include "datagram.h"
#include "pointerTo.h"

#include <string>

// This determines the size of the read buffer used to read UDP packets.  It
// places a limit on the maximum receivable size of a UDP packet (although it
// doesn't limit TCP packets at all).  However, there's no real reason this
// can't be set arbitrarily large, although there's not much point in making
// it larger than the system MTU, which also limits the maximum size of a UDP
// packet.
static const int maximum_udp_datagram = 1500;

/**
 * A specific kind of Datagram, especially for sending across or receiving
 * from a network.  It's different only in that it knows which Connection
 * and/or NetAddress it is to be sent to or was received from.
 */
class EXPCL_PANDA_NET NetDatagram : public Datagram {
PUBLISHED:
  NetDatagram();
  NetDatagram(const void *data, size_t size);
  NetDatagram(const Datagram &copy);
  NetDatagram(const NetDatagram &copy);
  void operator = (const Datagram &copy);
  void operator = (const NetDatagram &copy);

  virtual void clear();

  void set_connection(const PT(Connection) &connection);
  PT(Connection) get_connection() const;

  void set_address(const NetAddress &address);
  const NetAddress &get_address() const;

public:
  // We need these methods to make VC++ happy when we try to instantiate a
  // QueuedReturn<Datagram>.  They don't do anything useful.
  INLINE bool operator == (const NetDatagram &other) const;
  INLINE bool operator != (const NetDatagram &other) const;
  INLINE bool operator < (const NetDatagram &other) const;

private:
  PT(Connection) _connection;
  NetAddress _address;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    Datagram::init_type();
    register_type(_type_handle, "NetDatagram",
                  Datagram::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "netDatagram.I"

#endif
