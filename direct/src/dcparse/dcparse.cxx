// Filename: dcparse.cxx
// Created by:  drose (05Oct00)
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

#include <dcbase.h>
#include <dcFile.h>

int
main(int argc, char *argv[]) {
  if (argc < 2) {
    cerr <<
      "dcparse - a simple program to read one or more .dc files and report their\n"
      "contents to standard output.\n\n";
    return (1);
  }

  DCFile file;
  for (int i = 1; i < argc; i++) {
    if (!file.read(argv[i])) {
      return (1);
    }
  }

  if (!file.write(cout, "standard output")) {
    return (1);
  }

  long hash = file.get_hash();
  cout << "File hash is " << hash << "\n";

  return (0);
}
