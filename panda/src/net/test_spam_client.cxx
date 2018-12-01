/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_spam_client.cxx
 * @author drose
 * @date 2000-02-24
 */

#include "queuedConnectionManager.h"
#include "queuedConnectionReader.h"
#include "connectionWriter.h"
#include "netAddress.h"
#include "connection.h"
#include "netDatagram.h"
#include "clockObject.h"
#include "datagram_ui.h"
#include "thread.h"

int
main(int argc, char *argv[]) {
  if (argc != 3) {
    nout << "test_spam_client host port\n";
    exit(1);
  }

  std::string hostname = argv[1];
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
  std::cout << "Enter a datagram.\n";
  std::cin >> datagram;

  nout << "Read datagram " << datagram << "\n";
  datagram.dump_hex(nout);
  nout << "\n";

  int num_sent = 0;
  int num_received = 0;

  ClockObject *global_clock = ClockObject::get_global_clock();
  double last_reported_time = global_clock->get_real_time();
  static const double report_interval = 5.0;

  while (!lost_connection) {
    // Send the datagram.
    if (writer.send(datagram, c)) {
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
