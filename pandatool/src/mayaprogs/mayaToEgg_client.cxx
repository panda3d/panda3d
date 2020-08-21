/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mayaToEgg_client.cxx
 * @author cbrunner
 * @date 2009-11-09
 */

#include "mayaToEgg_client.h"

/**
 *
 */
MayaToEggClient::
MayaToEggClient() :
  SomethingToEgg("Maya", ".mb")
{
  qManager = new QueuedConnectionManager();
  qReader = new QueuedConnectionReader(qManager, 0);
  cWriter = new ConnectionWriter(qManager, 0);
  // We assume the server is local and on port 4242
  server.set_host("localhost", 4242);
}

int main(int argc, char *argv[]) {
  MayaToEggClient prog;
  // Open a connection to the server process
  PT(Connection) con = prog.qManager->open_TCP_client_connection(prog.server,0);
  if (con.is_null()) {
    nout << "Failed to open port to server process.\nMake sure maya2egg_server is running on localhost\n";
    exit(1);
  }

  // Add this connection to the readers list
  prog.qReader->add_connection(con);

  // Get the current working directory and make sure it's a string
  Filename cwd = ExecutionEnvironment::get_cwd();
  std::string s_cwd = (std::string)cwd.to_os_specific();
  NetDatagram datagram;

  // First part of the datagram is the argc
  datagram.add_uint8(argc);

  // Add the rest of the arguments as strings to the datagram
  int i;
  for (i = 0; i < argc; i++) {
    datagram.add_string(argv[i]);
  }

  // Lastly, add the current working dir as a string to the datagram
  datagram.add_string(s_cwd);

  // Send it and close the connection
  prog.cWriter->send(datagram, con);
  con->flush();
  while (true) {
    prog.qReader->data_available();
    if (prog.qManager->reset_connection_available()) {
      PT(Connection) connection;
      if (prog.qManager->get_reset_connection(connection)) {
        prog.qManager->close_connection(con);
        return 0;
      }
    }
    Thread::sleep(0.1);
  }
}
