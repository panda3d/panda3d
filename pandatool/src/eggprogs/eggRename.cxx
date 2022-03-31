/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggRename.cxx
 * @author masad
 * @date 2005-04-22
 */

#include "eggRename.h"

/**
 *
 */
EggRename::
EggRename() {
  set_program_brief("rename nodes in .egg files");
  set_program_description
    ("egg-rename reads one or more egg files and writes back with modified"
     "node names. ie. suppressing prefix from all the nodes' names. ");

  add_option
    ("strip_prefix", "name", 0,
     "strips out the prefix that is put on all nodes, by maya ext. ref",
     &EggRename::dispatch_vector_string, nullptr, &_strip_prefix);
}

/**
 *
 */
void EggRename::
run() {
  if (!_strip_prefix.empty()) {
    nout << "Stripping prefix from nodes.\n";
    int num_renamed = 0;
    int num_egg_files = 0;
    Eggs::iterator ei;
    for (ei = _eggs.begin(); ei != _eggs.end(); ++ei) {
      num_renamed += (*ei)->rename_nodes(_strip_prefix, true);
      ++num_egg_files;
    }
    nout << "  (" << num_renamed << " renamed.)\n";
  }

  write_eggs();
}


int main(int argc, char *argv[]) {
  EggRename prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
