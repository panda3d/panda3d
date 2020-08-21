/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file datagramGeneratorNet.h
 * @author drose
 * @date 2009-02-15
 */

#ifndef DATAGRAMGENERATORNET_H
#define DATAGRAMGENERATORNET_H

#include "pandabase.h"

#include "datagramGenerator.h"
#include "connectionReader.h"
#include "queuedReturn.h"
#include "pmutex.h"
#include "conditionVar.h"
#include "netDatagram.h"

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_NET, EXPTP_PANDA_NET, QueuedReturn<Datagram>);

/**
 * This class provides datagrams one-at-a-time as read directly from the net,
 * via a TCP connection.  If a datagram is not available, get_datagram() will
 * block until one is.
 */
class EXPCL_PANDA_NET DatagramGeneratorNet : public DatagramGenerator, public ConnectionReader, public QueuedReturn<Datagram> {
PUBLISHED:
  explicit DatagramGeneratorNet(ConnectionManager *manager, int num_threads);
  virtual ~DatagramGeneratorNet();

  // Inherited from DatagramGenerator
  virtual bool get_datagram(Datagram &data);
  virtual bool is_eof();
  virtual bool is_error();

protected:
  // Inherited from ConnectionReader
  virtual void receive_datagram(const NetDatagram &datagram);

  Mutex _dg_lock;
  ConditionVar _dg_received;  // notified when a new datagram is received.
  ConditionVar _dg_processed;  // notified when a new datagram is processed.
};

#include "datagramGeneratorNet.I"

#endif
