// Filename: test_spam_client.cxx
// Created by:  drose (24Feb00)
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

#include "queuedConnectionManager.h"
#include "queuedConnectionReader.h"
#include "connectionWriter.h"
#include "netAddress.h"
#include "connection.h"
#include "netDatagram.h"

#include "datagram_ui.h"

#include <prinrval.h>

int
main(int argc, char *argv[]) {
  if (argc != 3) {
    nout << "test_spam_client host port\n";
    exit(1);
  }

  string hostname = argv[1];
  int port = atoi(argv[2]);

  NetAddress host;
  if (!host.set_host(hostname, port)) {
    nout << "Unknown host: " << hostname << "\n";
  }

  QueuedConnectionManager cm;
  PT(Connection) c = cm.open_TCP_client_connection(host, 5000);

  if (c.is_null()) {
    nout << "No connection.\n";
    exit(1);
  }

  nout << "Successfully opened TCP connection to " << hostname
       << " on port " << port << "\n";

  QueuedConnectionReader reader(&cm, 10);
  reader.add_connection(c);
  ConnectionWriter writer(&cm, 10);

  bool lost_connection = false;

  NetDatagram datagram;
  cout << "Enter a datagram.\n";
  cin >> datagram;

  nout << "Read datagram " << datagram << "\n";
  datagram.dump_hex(nout);
  nout << "\n";

  int num_sent = 0;
  int num_received = 0;
  PRIntervalTime last_reported_time = PR_IntervalNow();
  PRIntervalTime report_interval = PR_SecondsToInterval(5);

  while (!lost_connection) {
    // Send the datagram.
    if (writer.send(datagram, c, host)) {
      num_sent++;
    }

    // Check for a lost connection.
    while (cm.reset_connection_available()) {
      PT(Connection) connection;
        if (cm.get_reset_connection(connection)) {
        nout << "Lost connection from "
             << connection->get_address() << "\n";
        cm.close_connection(connection);
        if (connection == c) {
          lost_connection = true;
        }
      }
    }

    // Now poll for new datagrams on the socket.
    if (reader.data_available()) {
      NetDatagram new_datagram;
      if (reader.get_data(new_datagram)) {
        num_received++;
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





