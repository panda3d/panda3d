/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vrmlTrans.cxx
 * @author drose
 * @date 2004-10-01
 */

#include "vrmlTrans.h"
#include "parse_vrml.h"

/**
 *
 */
VRMLTrans::
VRMLTrans() :
  WithOutputFile(true, true, false)
{
  // Indicate the extension name we expect the user to supply for output
  // files.
  _preferred_extension = ".wrl";

  set_program_brief("reads and writes VRML 2.0 files");
  set_program_description
    ("This program reads a VRML 2.0 file (.wrl) and writes an "
     "essentially equivalent .wrl file.  It is primarily useful for "
     "debugging the VRML parser that is part of the Pandatool library.");

  clear_runlines();
  add_runline("[opts] input.wrl > output.wrl");
  add_runline("[opts] input.wrl output.wrl");
  add_runline("[opts] -o output.wrl input.wrl");

  add_option
    ("o", "filename", 0,
     "Specify the filename to which the resulting .wrl file will be written.  "
     "If this option is omitted, the last parameter name is taken to be the "
     "name of the output file.",
     &VRMLTrans::dispatch_filename, &_got_output_filename, &_output_filename);
}


/**
 *
 */
void VRMLTrans::
run() {
  nout << "Reading " << _input_filename << "\n";

  VrmlScene *scene = parse_vrml(_input_filename);
  if (scene == nullptr) {
    nout << "Unable to read.\n";
    exit(1);
  }

  get_output() << *scene << "\n";
}


/**
 *
 */
bool VRMLTrans::
handle_args(ProgramBase::Args &args) {
  if (!check_last_arg(args, 1)) {
    return false;
  }

  if (args.empty()) {
    nout << "You must specify the .wrl file to read on the command line.\n";
    return false;

  } else if (args.size() != 1) {
    nout << "You must specify only one .wrl file to read on the command line.\n";
    return false;
  }

  _input_filename = args[0];

  return true;
}


int main(int argc, char *argv[]) {
  VRMLTrans prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
