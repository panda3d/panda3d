// Filename: test_pfstream.cxx
// Created by:  drose (04Nov02)
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
