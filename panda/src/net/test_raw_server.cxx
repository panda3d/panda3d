// Filename: test_raw_server.cxx
// Created by:  drose (20Jan04)
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

#include "pandabase.h"

#include "queuedConnectionManager.h"
#include "queuedConnectionListener.h"
#include "queuedConnectionReader.h"
#include "connectionWriter.h"
#include "netAddress.h"
#include "connection.h"
#include "netDatagram.h"

#include "pset.h"

int
main(int argc, char *argv[]) {
  if (argc != 2) {
    nout << "test_raw_server port\n";
    exit(1);
  }

  int port = atoi(argv[1]);

  QueuedConnectionManager cm;
  PT(Connection) rendezvous = cm.open_TCP_server_rendezvous(port, 5);

  if (rendezvous.is_null()) {
    nout << "Cannot grab port " << port << ".\n";
    exit(1);
  }

  nout << "Listening for connections on port " << port << "\n";

  QueuedConnectionListener listener(&cm, 0);
  listener.add_connection(rendezvous);

  typedef pset< PT(Connection) > Clients;
  Clients clients;

  QueuedConnectionReader reader(&cm, 0);
  ConnectionWriter writer(&cm, 0);
  reader.set_raw_mode(true);
  writer.set_raw_mode(true);

  bool shutdown = false;
  while (!shutdown) {
    // Check for new clients.
    while (listener.new_connection_available()) {
      PT(Connection) rv;
      NetAddress address;
      PT(Connection) new_connection;
      if (listener.get_new_connection(rv, address, new_connection)) {
        nout << "Got connection from " << address << "\n";
        reader.add_connection(new_connection);
        clients.insert(new_connection);
      }
    }

    // Check for reset clients.
    while (cm.reset_connection_available()) {
      PT(Connection) connection;
      if (cm.get_reset_connection(connection)) {
        nout << "Lost connection from "
             << connection->get_address() << "\n";
        clients.erase(connection);
        cm.close_connection(connection);
      }
    }

    // Process all available datagrams.
    while (reader.data_available()) {
      NetDatagram datagram;
      if (reader.get_data(datagram)) {
        string data = datagram.get_message();
        nout.write(data.data(), data.length());
        nout << flush;

        Clients::iterator ci;
        for (ci = clients.begin(); ci != clients.end(); ++ci) {
          writer.send(datagram, (*ci));
        }
      }
    }

    // Yield the timeslice before we poll again.
    PR_Sleep(PR_MillisecondsToInterval(100));
  }

  return (0);
}





