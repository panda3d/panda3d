// Filename: recentConnectionReader.h
// Created by:  drose (23Jun00)
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

#ifndef RECENTCONNECTIONREADER_H
#define RECENTCONNECTIONREADER_H

#include "pandabase.h"

#include "connectionReader.h"
#include "netDatagram.h"
#include "lightMutex.h"

////////////////////////////////////////////////////////////////////
//       Class : RecentConnectionReader
// Description : This flavor of ConnectionReader will read from its
//               sockets and retain only the single most recent
//               datagram for inspection by client code.  It's useful
//               particularly for reading telemetry-type data from UDP
//               sockets where you don't care about getting every last
//               socket, and in fact if the sockets are coming too
//               fast you'd prefer to skip some of them.
//
//               This class will always create one thread for itself.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_NET RecentConnectionReader : public ConnectionReader {
PUBLISHED:
  RecentConnectionReader(ConnectionManager *manager);
  virtual ~RecentConnectionReader();

  bool data_available();
  bool get_data(NetDatagram &result);
  bool get_data(Datagram &result);

protected:
  virtual void receive_datagram(const NetDatagram &datagram);

private:
  bool _available;
  Datagram _datagram;
  LightMutex _mutex;
};

#endif

