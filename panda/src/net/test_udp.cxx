/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_udp.cxx
 * @author drose
 * @date 2000-02-08
 */

#include "queuedConnectionManager.h"
#include "recentConnectionReader.h"
#include "connectionWriter.h"
#include "netAddress.h"
#include "connection.h"
#include "netDatagram.h"

#include "datagram_ui.h"

using std::cin;
using std::cout;

int
main(int argc, char *argv[]) {
  if (argc != 3) {
    nout << "test_udp host port\n";
    exit(1);
  }

  std::string hostname = argv[1];
  int port = atoi(argv[2]);

  NetAddress host;
  if (!host.set_host(hostname, port)) {
    nout << "Unknown host: " << hostname << "\n";
  }

  QueuedConnectionManager cm;
  PT(Connection) c = cm.open_UDP_connection(port);

  if (c.is_null()) {
    nout << "No connection.\n";
    exit(1);
  }

  nout << "Successfully opened UDP connection on port "
       << c->get_address().get_port() << " and IP "
       << c->get_address() << "\n";

  RecentConnectionReader reader(&cm);
  reader.add_connection(c);
  ConnectionWriter writer(&cm, 1);

  NetDatagram datagram;
  cout << "Enter a datagram.\n";
  cin >> datagram;

  bool lost_connection = false;

  while (!cin.fail() && !lost_connection) {
    // Send the datagram.
    writer.send(datagram, c, host);

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
    while (reader.data_available()) {
      if (reader.get_data(datagram)) {
        nout << "Got datagram " << datagram << "from "
             << datagram.get_address() << "\n";
        datagram.dump_hex(nout);
      }
    }

    cout << "\nEnter a datagram.\n";
    cin >> datagram;
  }
  nout << "Exiting.\n";

  return (0);
}
