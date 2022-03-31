/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lwoScan.cxx
 * @author drose
 * @date 2001-04-24
 */

#include "lwoScan.h"

#include "lwoInputFile.h"
#include "lwoChunk.h"
#include "config_lwo.h"

/**
 *
 */
LwoScan::
LwoScan() {
  clear_runlines();
  add_runline("[opts] input.lwo");

  set_program_brief("describe the contents of a Lightwave object file");
  set_program_description
    ("This program simply reads a Lightwave object file and dumps its "
     "contents to standard output.  It's mainly useful for debugging "
     "problems with lwo2egg.");
}

/**
 *
 */
void LwoScan::
run() {
  LwoInputFile in;
  if (!in.open_read(_input_filename)) {
    nout << "Unable to open " << _input_filename << "\n";
    exit(1);
  }

  PT(IffChunk) chunk = in.get_chunk();
  if (chunk == nullptr) {
    nout << "Unable to read file.\n";
  } else {
    while (chunk != nullptr) {
      chunk->write(std::cout, 0);
      chunk = in.get_chunk();
    }
  }
}

/**
 *
 */
bool LwoScan::
handle_args(ProgramBase::Args &args) {
  if (args.empty()) {
    nout << "You must specify the Lightwave object file to read on the command line.\n";
    return false;
  }
  if (args.size() != 1) {
    nout << "You may specify only one Lightwave object file to read on the command line.\n";
    return false;
  }

  _input_filename = args[0];

  return true;
}


int
main(int argc, char *argv[]) {
  init_liblwo();
  LwoScan prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
