// Filename: test_egg.cxx
// Created by:  drose (16Jan99)
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

#include "eggData.h"
#include "notify.h"


int
main(int argc, char *argv[]) {
  if (argc != 2) {
    nout << "Specify an egg file to load.\n";
    exit(1);
  }
  const char *egg_filename = argv[1];

  EggData data;
  data.set_coordinate_system(CS_default);

  if (data.read(egg_filename)) {
    data.load_externals("");
    data.write_egg(cout);
  } else {
    nout << "Errors.\n";
  }
  return (0);
}

