/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fake_http_server.cxx
 * @author drose
 * @date 2002-12-10
 */

#include "pandabase.h"

#include "queuedConnectionManager.h"
#include "queuedConnectionListener.h"
#include "queuedConnectionReader.h"
#include "connectionWriter.h"
#include "netAddress.h"
#include "connection.h"
#include "netDatagram.h"
#include "pmap.h"

#include <ctype.h>

using std::string;

QueuedConnectionManager cm;
QueuedConnectionReader reader(&cm, 10);
ConnectionWriter writer(&cm, 10);

class ClientState {
public:
  ClientState(Connection *client);
  void receive_data(const Datagram &data);
  void receive_line(string line);

  Connection *_client;
  string _received;
};

ClientState::
ClientState(Connection *client) {
  _client = client;
}

void ClientState::
receive_data(const Datagram &data) {
  _received += data.get_message();
  size_t next = 0;
  size_t newline = _received.find('\n', next);
  while (newline != string::npos) {
    size_t last = next;
    next = newline + 1;
    if (newline > 0 && _received[newline - 1] == '\r') {
      newline--;
    }
    receive_line(_received.substr(last, newline - last));
    if (next < _received.size() && _received[next] == '\r') {
      next++;
    }
    newline = _received.find('\n', next);
  }
  _received = _received.substr(next);
}

void ClientState::
receive_line(string line) {
  std::cerr << "received: " << line << "\n";
  // trim trailing whitespace.
  size_t size = line.size();
  while (size > 0 && isspace(line[size - 1])) {
    size--;
  }
  if (size != line.size()) {
    line = line.substr(0, size);
  }

  /*
  if (line.empty()) {
    // Start to honor the request, as if we cared.
    Datagram dg;
    dg.append_data("HTTP/1.1 200 OK\r\n");
    writer.send(dg, _client);
    // Close the connection!
    cm.close_connection(_client);
  }
  */
}


int
main(int argc, char *argv[]) {
  if (argc != 2) {
    nout << "fake_http_server port\n";
    exit(1);
  }

  int port = atoi(argv[1]);

  PT(Connection) rendezvous = cm.open_TCP_server_rendezvous(port, 5);

  if (rendezvous.is_null()) {
    nout << "Cannot grab port " << port << ".\n";
    exit(1);
  }

  nout << "Listening for connections on port " << port << "\n";

  QueuedConnectionListener listener(&cm, 1);
  listener.add_connection(rendezvous);

  typedef pmap< PT(Connection), ClientState > Clients;
  Clients clients;

  reader.set_raw_mode(1);
  writer.set_raw_mode(1);

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
        clients.insert(Clients::value_type(new_connection, ClientState(new_connection)));
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
        PT(Connection) client = datagram.get_connection();
        Clients::iterator ci = clients.find(client);
        if (ci == clients.end()) {
          nout << "Received data from unexpected client " << (void *)client
               << "\n";
        } else {
          ClientState &state = (*ci).second;
          state.receive_data(datagram);
        }
      }
    }
  }

  return (0);
}
