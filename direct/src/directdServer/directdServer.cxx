// Filename: directd.cxx
// Created by:  skyler 2002.04.08
// Based on test_tcp_*.* by drose.
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

#include "directd.h"

int
main(int argc, char *argv[]) {
  if (argc > 1 && strcmp(argv[1], "--help")==0) {
    cerr<<"directd [-c <host>] <port>\n"
    "    -c        run as client (else run as server).\n"
    "    host      e.g. localhost\n"
    "    port      default 8001\n";
    return 1;
  }

  cerr<<"directd"<<endl;
  DirectD directd;
  if (argc >= 3) {
    string host=argv[argc-2];
    directd.set_host_name(host);
  }
  char run_as=' ';
  if (argc > 1) {
    directd.set_port(atoi(argv[argc-1]));
    if (strlen(argv[1]) > 1 && argv[1][0] == '-') {
      run_as=argv[1][1];
    }
  }
  switch (run_as) {
  case 's':
    directd.run_server();
    break;
  case 'c':
  default:
    directd.run_client();
    break;
  }
  
  return 0;
}
