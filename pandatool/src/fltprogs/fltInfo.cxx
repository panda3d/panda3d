// Filename: fltInfo.cxx
// Created by:  drose (05Sep01)
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

#include "fltInfo.h"

#include "fltHeader.h"
#include "indent.h"

////////////////////////////////////////////////////////////////////
//     Function: FltInfo::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
FltInfo::
FltInfo() {
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


////////////////////////////////////////////////////////////////////
//     Function: FltInfo::run
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: FltInfo::list_hierarchy
//       Access: Protected
//  Description: Recursively lists the flt file's hierarchy in a
//               meaningful way.
////////////////////////////////////////////////////////////////////
void FltInfo::
list_hierarchy(FltRecord *record, int indent_level) {
  // Maybe in the future we can do something fancier here.
  record->write(cout, indent_level);
}


////////////////////////////////////////////////////////////////////
//     Function: FltInfo::handle_args
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
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
