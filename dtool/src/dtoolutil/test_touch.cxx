// Filename: test_pfstream.cxx
// Created by:  drose (04Nov02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "dtoolbase.h"
#include "filename.h"

int 
main(int argc, char *argv[]) {
  if (argc < 2) {
    cout << "test_touch filename [filename ... ]\n";
    return (1);
  }

  for (int i = 1; i < argc; i++) {
    Filename filename(argv[i]);
    filename.touch();
  }

  return (0);
}
