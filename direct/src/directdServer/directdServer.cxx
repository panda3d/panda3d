// Filename: directdServer.cxx
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

#include "directdServer.h"

DirectDServer::DirectDServer() {
}

DirectDServer::~DirectDServer() {
}

void
DirectDServer::handle_command(const string& cmd) {
  nout<<"DirectDServer::handle_command: "<<cmd<<", size="<<cmd.size()<<endl;
  if (cmd.size()==1) {
    switch (cmd[0]) {
    case 'k':
      kill_app(0);
      break;
    case 'q':
      _shutdown=true;
      break;
    default:
      cerr<<"unknown command: "<<cmd<<endl;
      break;
    }
  } else {
    switch (cmd[0]) {
    case 'k':
      if (cmd[1]=='a') {
        kill_all();
      } else {
        int index = atoi(cmd.substr(1, string::npos).c_str());
        kill_app(index);
      }
      break;
    case '!': {
      string c=cmd.substr(1, string::npos);
      //read_command(c);
      start_app(c);
      }
      break;
    default:
      start_app(cmd);
      break;
    }
  }
}

void
DirectDServer::read_command(string& cmd) {
  try {
    ifstream f;
    f.open("directdCommand", ios::in | ios::binary);
    stringstream ss;
    const buf_size=512;
    char buf[buf_size];
    f.getline(buf, buf_size);
    if (f.gcount() > 0) {
      cmd = buf;
      cerr<<"read_command "<<cmd<<endl;
    }
    f.close();
  } catch (...) {
    // This could be bad, I suppose.  But we're going to throw out
    // any exceptions that happen during the above read.
    cerr<<"DirectD::read_command() exception."<<endl;
  }
}

void
DirectDServer::run_server(int port) {
  nout<<"server"<<endl;
  
  listen_to(port);

  while (!_shutdown) {
    check_for_new_clients();
    check_for_lost_connection();
    check_for_datagrams();

    // Yield the timeslice before we poll again.
    PR_Sleep(PR_MillisecondsToInterval(200));
  }
}

int
main(int argc, char *argv[]) {
  if (argc > 1 && strcmp(argv[1], "--help")==0) {
    cerr<<"directd [<port>]\n"
    "    port      default 8001\n";
    return 1;
  }

  cerr<<"directdServer "<<__DATE__<<" "<<__TIME__<<endl;
  int port=8001;
  if (argc > 1) {
    port=(atoi(argv[argc-1]));
  }
  DirectDServer directd;
  directd.run_server(port);
  
  return 0;
}
