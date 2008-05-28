// Filename: test_spam_server.cxx
// Created by:  drose (24Feb00)
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

#include "datagram_ui.h"
#include "clockObject.h"
#include "thread.h"

#include "pset.h"
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

int
main(int argc, char *argv[]) {
  if (argc != 2) {
    nout << "test_spam_server port\n";
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

  QueuedConnectionListener listener(&cm, 1);
  listener.add_connection(rendezvous);

  typedef pset< PT(Connection) > Clients;
  Clients clients;

  QueuedConnectionReader reader(&cm, 10);
  ConnectionWriter writer(&cm, 10);

  int num_sent = 0;
  int num_received = 0;

  ClockObject *global_clock = ClockObject::get_global_clock();
  double last_reported_time = global_clock->get_real_time();
  static const double report_interval = 5.0;

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
        num_received++;
        Clients::iterator ci;
        for (ci = clients.begin(); ci != clients.end(); ++ci) {
          if (writer.send(datagram, (*ci))) {
            num_sent++;
          }
        }
      }
    }

    double now = global_clock->get_real_time();
    if ((now - last_reported_time) > report_interval) {
      nout << "Sent " << num_sent << ", received "
           << num_received << " datagrams.\n";
      last_reported_time = now;
    }

    // Yield the timeslice before we poll again.
    Thread::sleep(0.001);
  }

  return (0);
}





