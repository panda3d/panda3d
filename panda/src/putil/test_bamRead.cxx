// Filename: test_bamRead.cxx
// Created by:  jason (13Jun00)
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

#include "pandabase.h"
#include "pnotify.h"

#include <ipc_file.h>
#include "test_bam.h"
#include "bamReader.h"

int main(int argc, char* argv[])
{
  string test_file = "bamTest.out";
  datagram_file stream(test_file);

  stream.open(file::FILE_READ);
  BamReader manager(&stream);

  manager.init();

  PointerTo<Parent> dad = DCAST(Parent, manager.read_object());
  PointerTo<Parent> mom = DCAST(Parent, manager.read_object());
  PointerTo<Child> bro = DCAST(Child, manager.read_object());
  PointerTo<Child> sis = DCAST(Child, manager.read_object());

  manager.resolve();

  dad->print_relationships();
  nout << endl;
  mom->print_relationships();
  nout << endl;
  bro->print_relationships();
  nout << endl;
  sis->print_relationships();

  return 1;
}
