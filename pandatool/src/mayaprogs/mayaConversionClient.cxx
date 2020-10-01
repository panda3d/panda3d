/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mayaConversionClient.cxx
 * @author Derzsi Dániel
 * @date 2020-10-01
 */

#include "mayaConversionClient.h"
#include "mayaConversionServer.h"

/**
 * Initializes the Maya conversion client.
 */
MayaConversionClient::
MayaConversionClient()
{
  _qManager = new QueuedConnectionManager();
  _qReader = new QueuedConnectionReader(_qManager, 0);
  _cWriter = new ConnectionWriter(_qManager, 0);
}

/**
 * Cleans up the Maya conversison client's pending connections.
 */
MayaConversionClient::
~MayaConversionClient() {
  close();
}

/**
 * Attempts to connect to a Maya conversion server.
 * If a connection is already active, it will be removed.
 *
 * Returns true if the connection was created successfully.
 */
bool MayaConversionClient::
connect(NetAddress server) {
  if (_conn) {
    // Remove this connection from the readers list
    _qReader->remove_connection(_conn);
    _conn = nullptr;
  }

  // Attempt to open a connection
  _conn = _qManager->open_TCP_client_connection(server, 0);

  if (!_conn || !_conn.is_null()) {
    // This connection could not be opened
    return false;
  }

  // Add this connection to the readers list
  _qReader->add_connection(_conn);
  return true;
}

/**
 * Sends a conversion workload to the Maya conversion server.
 * It will be processed as soon as the server's queue is empty.
 *
 * Specify the working directory in where the models are located,
 * the command line arguments, and the conversion type.
 *
 * Returns true if the workload has been successfully sent.
 */
bool MayaConversionClient::
queue(Filename working_directory, int argc, char *argv[], MayaConversionServer::ConversionType conversion_type) {
  if (!_conn) {
    return false;
  }

  std::string s_cwd = (std::string) working_directory.to_os_specific();
  NetDatagram datagram;

  // First part of the datagram is the argc
  datagram.add_uint8(argc);

  // Add the rest of the arguments as strings to the datagram
  for (int i = 0; i < argc; i++) {
    datagram.add_string(argv[i]);
  }

  // Add the current working dir as a string to the datagram
  datagram.add_string(s_cwd);

  // Lastly, add the conversion type
  datagram.add_uint8(conversion_type);

  // Send it and close the connection
  return _cWriter->send(datagram, _conn) && _conn->flush();
}

/**
 * Closes the current connection to the Maya conversion server,
 * waiting for all currently queued requests to finish.
 *
 * Does nothing if the connection has not been made yet.
 */
void MayaConversionClient::
close() {
  if (!_conn) {
    return;
  }

  while (true) {
    _qReader->data_available();

    if (_qManager->reset_connection_available()) {
      PT(Connection) connection;

      if (_qManager->get_reset_connection(connection)) {
        _qManager->close_connection(_conn);
        _conn = nullptr;
        return;
      }
    }

    Thread::sleep(0.1);
  }
}

/**
 * The entrypoint to this Maya conversion client.
 *
 * Connects to the default server at port 4242, queues
 * one conversion request and waits for its completion.
 */
int MayaConversionClient::
main(int argc, char *argv[], MayaConversionServer::ConversionType conversion_type) {
  NetAddress server;

  // We assume the server is local and on port 4242
  server.set_host("localhost", 4242);

  if (!connect(server)) {
    nout << "Failed to open port to server process.\n"
         << "Make sure maya2egg -server or egg2maya -server is running on localhost!\n";
    return 1;
  }

  if (!queue(ExecutionEnvironment::get_cwd(), argc, argv, conversion_type)) {
    nout << "Failed to send workload to server process.\n";
    return 1;
  }

  close();
  return 0;
}
