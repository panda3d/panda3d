// Filename: xFileTrans.cxx
// Created by:  drose (03Oct04)
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

#include "xFileTrans.h"
#include "xFile.h"

////////////////////////////////////////////////////////////////////
//     Function: XFileTrans::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
XFileTrans::
XFileTrans() :
  WithOutputFile(true, false, true)
{
  // Indicate the extension name we expect the user to supply for
  // output files.
  _preferred_extension = ".x";

  set_program_description
    ("This program reads a DirectX retained-mode file (.x) and writes an "
     "essentially equivalent .x file.  It is primarily useful for "
     "debugging the X file parser that is part of the Pandatool library.");

  clear_runlines();
  add_runline("[opts] input.x output.x");
  add_runline("[opts] -o output.x input.x");

  add_option
    ("o", "filename", 0,
     "Specify the filename to which the resulting .x file will be written.  "
     "If this option is omitted, the last parameter name is taken to be the "
     "name of the output file.",
     &XFileTrans::dispatch_filename, &_got_output_filename, &_output_filename);
}


////////////////////////////////////////////////////////////////////
//     Function: XFileTrans::run
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void XFileTrans::
run() {
  nout << "Reading " << _input_filename << "\n";

  XFile file;
  if (!file.read(_input_filename)) {
    nout << "Unable to read.\n";
    exit(1);
  }

  if (!file.write(get_output())) {
    nout << "Unable to write.\n";
    exit(1);
  }
}


////////////////////////////////////////////////////////////////////
//     Function: XFileTrans::handle_args
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool XFileTrans::
handle_args(ProgramBase::Args &args) {
  if (!check_last_arg(args, 1)) {
    return false;
  }

  if (args.empty()) {
    nout << "You must specify the .x file to read on the command line.\n";
    return false;

  } else if (args.size() != 1) {
    nout << "You must specify only one .x file to read on the command line.\n";
    return false;
  }

  _input_filename = args[0];

  return true;
}


int main(int argc, char *argv[]) {
  XFileTrans prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
