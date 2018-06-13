/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_bamRead.cxx
 * @author jason
 * @date 2000-06-13
 */

#include "pandabase.h"
#include "pnotify.h"

#include "test_bam.h"
#include "bamReader.h"
#include "datagramInputFile.h"

int main(int argc, char* argv[])
{
  std::string test_file = "bamTest.out";
  DatagramInputFile stream;
  bool success = stream.open(test_file);
  nassertr(success, 1);

  BamReader manager(&stream);

  manager.init();

  PointerTo<Parent> dad = DCAST(Parent, manager.read_object());
  PointerTo<Parent> mom = DCAST(Parent, manager.read_object());
  PointerTo<Child> bro = DCAST(Child, manager.read_object());
  PointerTo<Child> sis = DCAST(Child, manager.read_object());

  manager.resolve();

  dad->print_relationships();
  nout << std::endl;
  mom->print_relationships();
  nout << std::endl;
  bro->print_relationships();
  nout << std::endl;
  sis->print_relationships();

  return 0;
}
