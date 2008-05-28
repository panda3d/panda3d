// Filename: test_tcp_server.cxx
// Created by:  drose (09Feb00)
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

#include "pandabase.h"

#include "queuedConnectionManager.h"
#include "queuedConnectionListener.h"
#include "queuedConnectionReader.h"
#include "connectionWriter.h"
#include "netAddress.h"
#include "connection.h"
#include "netDatagram.h"
#include "thread.h"
#include "datagram_ui.h"

#include "pset.h"

int
main(int argc, char *argv[]) {
  if (argc != 2) {
    nout << "test_tcp_server port\n";
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

  QueuedConnectionReader reader(&cm, 1);
  ConnectionWriter writer(&cm, 1);

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
        nout << "Got datagram " << datagram << "from "
             << datagram.get_address() << ", sending to "
             << clients.size() << " clients.\n";
        datagram.dump_hex(nout);

        Clients::iterator ci;
        for (ci = clients.begin(); ci != clients.end(); ++ci) {
          writer.send(datagram, (*ci));
        }

        if (datagram.get_length() <= 1) {
          /*
          // An empty datagram means to close the connection.
          PT(Connection) connection = datagram.get_connection();
          if (connection.is_null()) {
            nout << "Empty datagram from a null connection.\n";
          } else {
            nout << "Closing connection from "
                 << connection->get_address() << "\n";
            clients.erase(connection);
            cm.close_connection(connection);
            nout << "Closed " << connection << "\n";
          }
          */

          // No, an empty datagram means to shut down the server.
          shutdown = true;
        }
      }
    }

    // Yield the timeslice before we poll again.
    Thread::sleep(0.1);
  }

  return (0);
}





