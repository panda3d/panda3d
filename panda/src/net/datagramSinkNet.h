// Filename: datagramSinkNet.h
// Created by:  drose (15Feb09)
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

#ifndef DATAGRAMSINKNET_H
#define DATAGRAMSINKNET_H

#include "pandabase.h"

#include "datagramSink.h"
#include "connectionWriter.h"

////////////////////////////////////////////////////////////////////
//       Class : DatagramSinkNet
// Description : This class accepts datagrams one-at-a-time and sends
//               them over the net, via a TCP connection.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_NET DatagramSinkNet : public DatagramSink, public ConnectionWriter {
PUBLISHED:
  DatagramSinkNet(ConnectionManager *manager, int num_threads);

  INLINE void set_target(Connection *connection);
  INLINE Connection *get_target() const;

  // Inherited from DatagramSink
  virtual bool put_datagram(const Datagram &data);
  virtual bool is_error();
  virtual void flush();

private:
  PT(Connection) _target;
};

#include "datagramSinkNet.I"

#endif
