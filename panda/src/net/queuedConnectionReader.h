// Filename: queuedConnectionReader.h
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

#ifndef QUEUEDCONNECTIONREADER_H
#define QUEUEDCONNECTIONREADER_H

#include "pandabase.h"

#include "connectionReader.h"
#include "netDatagram.h"
#include "queuedReturn.h"

#include <prlock.h>
#include "pdeque.h"

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, QueuedReturn<NetDatagram>);

////////////////////////////////////////////////////////////////////
//       Class : QueuedConnectionReader
// Description : This flavor of ConnectionReader will read from its
//               sockets and queue up all of the datagrams read for
//               later receipt by the client code.  This class is
//               useful for client code that doesn't want to deal with
//               threading and is willing to poll for datagrams at its
//               convenience.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA QueuedConnectionReader : public ConnectionReader,
                               public QueuedReturn<NetDatagram> {
PUBLISHED:
  QueuedConnectionReader(ConnectionManager *manager, int num_threads);
  virtual ~QueuedConnectionReader();

  bool data_available();
  bool get_data(NetDatagram &result);
  bool get_data(Datagram &result);

protected:
  virtual void receive_datagram(const NetDatagram &datagram);

#ifdef SIMULATE_NETWORK_DELAY
PUBLISHED:
  void start_delay(double min_delay, double max_delay);
  void stop_delay();

private:
  void get_delayed();
  void delay_datagram(const NetDatagram &datagram);

  class DelayedDatagram {
  public:
    double _reveal_time;
    NetDatagram _datagram;
  };
    
  PRLock *_dd_mutex;
  typedef pdeque<DelayedDatagram> Delayed;
  Delayed _delayed;
  bool _delay_active;
  double _min_delay, _delay_variance;

#endif  // SIMULATE_NETWORK_DELAY
};

#endif

