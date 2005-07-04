// Filename: test_pfstream.cxx
// Created by:  drose (31Jul02)
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

#include "dtoolbase.h"
#include "pfstream.h"

int 
main(int argc, char *argv[]) {
  if (argc < 2) {
    cout << "test_pfstream command-line\n";
    return (1);
  }

  // Build one command out of the arguments.
  string cmd;
  cmd = argv[1];
  for (int i = 2; i < argc; i++) {
    cmd += " ";
    cmd += argv[i];
  }

  cout << "Executing command:\n" << cmd << "\n";
  
  IPipeStream in(cmd);

  char c;
  c = in.get();
  while (in && !in.fail() && !in.eof()) {
    cout.put(toupper(c));
    c = in.get();
  }

  return (0);
}
