// Filename: test_bamRead.cxx
// Created by:  jason (13Jun00)
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

#include "pandabase.h"
#include "notify.h"

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
