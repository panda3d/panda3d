// Filename: lwoScan.cxx
// Created by:  drose (24Apr01)
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

#include "lwoScan.h"

#include "lwoInputFile.h"
#include "lwoChunk.h"
#include "config_lwo.h"

////////////////////////////////////////////////////////////////////
//     Function: LwoScan::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
LwoScan::
LwoScan() {
  clear_runlines();
  add_runline("[opts] input.lwo");

  set_program_description
    ("This program simply reads a Lightwave object file and dumps its "
     "contents to standard output.  It's mainly useful for debugging "
     "problems with lwo2egg.");
}

////////////////////////////////////////////////////////////////////
//     Function: LwoScan::run
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void LwoScan::
run() {
  LwoInputFile in;
  if (!in.open_read(_input_filename)) {
    nout << "Unable to open " << _input_filename << "\n";
    exit(1);
  }

  PT(IffChunk) chunk = in.get_chunk();
  if (chunk == (IffChunk *)NULL) {
    nout << "Unable to read file.\n";
  } else {
    while (chunk != (IffChunk *)NULL) {
      chunk->write(cout, 0);
      chunk = in.get_chunk();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LwoScan::handle_args
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
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
