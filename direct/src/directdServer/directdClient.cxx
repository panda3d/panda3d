/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file directdClient.cxx
 * @author skyler
 * @date 2002-04-08
 */

#include "directdClient.h"

using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::string;

DirectDClient::DirectDClient() {
}

DirectDClient::~DirectDClient() {
}

void
DirectDClient::cli_command(const string& cmd) {
  cerr<<"command "<<cmd<<endl;
  if (cmd[0]==':') {
    // ...connect to host.
    cerr<<"Local command "<<std::flush;
    string code;
    cin >> code;
    string host;
    cin >> host;
    int port;
    cin >> port;
    cerr<<"connect ("<<code<<") to "<<host<<" port "<<port<<endl;
    connect_to(host, port);
  } else {
    send_command(cmd);
    if (cmd[0] == 'q' && cmd.size()==1) {
      // ...user entered quit command.
      exit(0);
    }
  }
}

void
DirectDClient::run_client(const string& host, int port) {
  nout<<"client"<<endl;

  connect_to(host, port);

  while (!cin.fail() && _connections.size()!=0) {
    cout << "directd send: " << std::flush;
    string d;
    cin >> d;
    cli_command(d);

    check_for_lost_connection();
    check_for_datagrams();
  }
  nout << "Exiting\n";
}

int
main(int argc, char *argv[]) {
  if (argc > 1 && strcmp(argv[1], "--help")==0) {
    cerr<<"directd [[<host>] <port>]\n"
    "    host      default localhost\n"
    "    port      default 8001\n";
    return 1;
  }

  cerr<<"directdClient "<<__DATE__<<" "<<__TIME__<<endl;
  string host="localhost";
  int port=8001;
  if (argc >= 3) {
    host=argv[argc-2];
  }
  if (argc > 1) {
    port=(atoi(argv[argc-1]));
  }
  DirectDClient directd;
  directd.run_client(host, port);

  return 0;
}
