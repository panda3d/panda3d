// Filename: test_spam_server.cxx
// Created by:  drose (24Feb00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
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

#include <prinrval.h>

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
  PRIntervalTime last_reported_time = PR_IntervalNow();
  PRIntervalTime report_interval = PR_SecondsToInterval(5);

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

    PRIntervalTime now = PR_IntervalNow();
    if ((PRIntervalTime)(now - last_reported_time) > report_interval) {
      nout << "Sent " << num_sent << ", received "
           << num_received << " datagrams.\n";
      last_reported_time = now;
    }

    // Yield the timeslice before we poll again.
    //    PR_Sleep(PR_MillisecondsToInterval(1));
  }

  return (0);
}





