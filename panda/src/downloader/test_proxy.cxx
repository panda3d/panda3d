// Filename: test_proxy.cxx
// Created by:  drose (29Aug02)
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
#include "string_utils.h"
#include "queuedConnectionManager.h"
#include "queuedConnectionReader.h"
#include "connectionWriter.h"
#include "connection.h"
#include "pointerTo.h"
#include "datagram.h"

string proxy_server;
int proxy_port;
string data_server;
string url;

class HTTPClient {
public:
  HTTPClient();
  bool connect(const string &server, int port);
  bool send(const string &data);
  bool read(string &result);
  bool test_connection();
  void close();
  bool is_closed() const;

  QueuedConnectionManager _manager;
  QueuedConnectionReader _reader;
  ConnectionWriter _writer;
  PT(Connection) _connection;
};

HTTPClient::
HTTPClient() :
  _reader(&_manager, 0),
  _writer(&_manager, 0)
{
  _reader.set_raw_mode(true);
  _writer.set_raw_mode(true);
}

bool HTTPClient::
connect(const string &server, int port) {
  test_connection();
  nassertr(_connection.is_null(), false);

  _connection = _manager.open_TCP_client_connection(server, port, 1000);
  if (_connection.is_null()) {
    return false;
  }

  _reader.add_connection(_connection);
  return true;
}

bool HTTPClient::
send(const string &data) {
  test_connection();
  Datagram dg(data);
  return _writer.send(dg, _connection);
}

bool HTTPClient::
read(string &result) {
  test_connection();
  bool any_data = false;

  Datagram dg;
  while (_reader.data_available()) {
    if (_reader.get_data(dg)) {
      result += dg.get_message();
      any_data = true;
    }
  }

  return any_data;
}

bool HTTPClient::
test_connection() {
  bool okflag = true;
  while (_manager.reset_connection_available()) {
    PT(Connection) c;
    if (_manager.get_reset_connection(c)) {
      cerr << "lost connection: " << (void *)c << "\n";
      _manager.close_connection(_connection);
      _connection = NULL;
      okflag = false;
    }
  }

  return okflag;
}

void HTTPClient::
close() {
  test_connection();
  if (!_connection.is_null()) {
    _manager.close_connection(_connection);
    _connection = NULL;
  }
}

bool HTTPClient::
is_closed() const {
  return (_connection.is_null());
}

int 
main(int argc, char *argv[]) {
  bool okflag = false;

  if (argc == 4 || argc == 5) {
    if (string_to_int(argv[2], proxy_port)) {
      proxy_server = argv[1];
      data_server = argv[3];
      if (argc >= 5) {
        url = argv[4];
      }
      okflag = true;
    }
  }

  if (!okflag) {
    cerr << "test_proxy proxy_server proxy_port data_server [url]\n";
    exit(1);
  }

  HTTPClient client;
  if (!client.connect(proxy_server, proxy_port)) {
    cerr << "Unable to connect to " << proxy_server << ":" << proxy_port << "\n";
    exit(1);
  }

  ostringstream request_strm;

  if (!url.empty()) {
    // Send a URL request to the proxy server.
    request_strm
      << "CONNECT " << data_server << " HTTP/1.0\n"
      << "\n"
      << "GET " << url << " HTTP/1.0\n"
      << "\n";

    /*
    request_strm
      << "GET " << url << " HTTP/1.1\n"
      << "Host: " << data_server << "\n"
      << "\n";*/
  } else {
    // Send a raw CONNECT request to the proxy server.
    request_strm
      << "CONNECT " << data_server << " HTTP/1.0\n"
      << "\n";
  }

  string request = request_strm.str();
  cerr << request;
  
  if (!client.send(request)) {
    cerr << "Error transmitting to proxy server.\n";
    exit(1);
  }

  cerr << "waiting.\n";

  while (!client.is_closed()) {
    string result;
    while (client.read(result)) {
      cerr << result;
      result = "";
    }
  }

  cerr << "terminating.\n";
  client.close();

  return (0);
}

