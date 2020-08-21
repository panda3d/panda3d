/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fltInfo.cxx
 * @author drose
 * @date 2001-09-05
 */

#include "fltInfo.h"

#include "fltHeader.h"
#include "indent.h"

/**
 *
 */
FltInfo::
FltInfo() {
  set_program_brief("describe the contents of a MultiGen .flt file");
  set_program_description
    ("This program reads a MultiGen OpenFlight (.flt) file and reports "
     "some interesting things about its contents.");

  clear_runlines();
  add_runline("[opts] input.flt");

  add_option
    ("ls", "", 0,
     "List the hierarchy in the flt file.",
     &FltInfo::dispatch_none, &_list_hierarchy);
}


/**
 *
 */
void FltInfo::
run() {
  PT(FltHeader) header = new FltHeader(_path_replace);

  nout << "Reading " << _input_filename << "\n";
  FltError result = header->read_flt(_input_filename);
  if (result != FE_ok) {
    nout << "Unable to read: " << result << "\n";
    exit(1);
  }

  if (header->check_version()) {
    nout << "Version is " << header->get_flt_version() / 100.0 << "\n";
  }

  if (_list_hierarchy) {
    list_hierarchy(header, 0);
  }
}

/**
 * Recursively lists the flt file's hierarchy in a meaningful way.
 */
void FltInfo::
list_hierarchy(FltRecord *record, int indent_level) {
  // Maybe in the future we can do something fancier here.
  record->write(std::cout, indent_level);
}


/**
 *
 */
bool FltInfo::
handle_args(ProgramBase::Args &args) {
  if (args.empty()) {
    nout << "You must specify the .flt file to read on the command line.\n";
    return false;

  } else if (args.size() != 1) {
    nout << "You must specify only one .flt file to read on the command line.\n";
    return false;
  }

  _input_filename = args[0];

  return true;
}


int main(int argc, char *argv[]) {
  FltInfo prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
