/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_raw_server.cxx
 * @author drose
 * @date 2004-01-20
 */

#include "pandabase.h"

#include "queuedConnectionManager.h"
#include "queuedConnectionListener.h"
#include "queuedConnectionReader.h"
#include "connectionWriter.h"
#include "netAddress.h"
#include "connection.h"
#include "netDatagram.h"
#include "thread.h"
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
        std::string data = datagram.get_message();
        nout.write(data.data(), data.length());
        nout << std::flush;

        Clients::iterator ci;
        for (ci = clients.begin(); ci != clients.end(); ++ci) {
          writer.send(datagram, (*ci));
        }
      }
    }

    // Yield the timeslice before we poll again.
    Thread::sleep(0.1);
  }

  return (0);
}
