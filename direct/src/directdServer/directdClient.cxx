// Filename: directdClient.cxx
// Created by:  skyler 2002.04.08
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

#include "directdClient.h"

DirectDClient::DirectDClient() {
}

DirectDClient::~DirectDClient() {
}

void
DirectDClient::cli_command(const string& cmd) {
  cerr<<"command "<<cmd<<endl;
  if (cmd[0]==':') {
    // ...connect to host.
    cerr<<"Local command "<<flush;
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
    cout << "directd send: " << flush;
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
