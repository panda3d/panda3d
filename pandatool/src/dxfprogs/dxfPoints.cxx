// Filename: dxfPoints.cxx
// Created by:  drose (04May04)
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

#include "dxfPoints.h"
#include "pystub.h"

////////////////////////////////////////////////////////////////////
//     Function: DXFPoints::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DXFPoints::
DXFPoints() :
  WithOutputFile(true, true, false)
{
  // Indicate the extension name we expect the user to supply for
  // output files.
  _preferred_extension = ".txt";

  set_program_brief("extract points from AutoCAD .dxf files");
  set_program_description
    ("This program reads an AutoCAD .dxf file and generates a simple "
     "list of all the points contained within it, one per line, to a "
     "text file, or to standard output.");

  clear_runlines();
  add_runline("[opts] input.dxf > output.txt");
  add_runline("[opts] -o output.txt input.dxf");
  add_runline("[opts] input.dxf output.txt");
}


////////////////////////////////////////////////////////////////////
//     Function: DXFPoints::run
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void DXFPoints::
run() {
  // Invoke the DXFFile base class to process the input file.
  process(_input_filename);
}

////////////////////////////////////////////////////////////////////
//     Function: DXFPoints::done_entity
//       Access: Public, Virtual
//  Description: This is inherited from DXFFile, and gets called as
//               each entity (face, line, whatever) has finished
//               processing.
////////////////////////////////////////////////////////////////////
void DXFPoints::
done_entity() {
  if (_entity == EN_point) {
    get_output() << _p << "\n";

  } else if (_entity == EN_insert) {
    ocs_2_wcs();
    get_output() << _p << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXFPoints::handle_args
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool DXFPoints::
handle_args(ProgramBase::Args &args) {
  if (args.empty()) {
    nout << "You must specify the .dxf file to read on the command line.\n";
    return false;

  } else if (args.size() != 1) {
    nout << "You must specify only one .dxf file to read on the command line.\n";
    return false;
  }

  _input_filename = args[0];

  return true;
}


int main(int argc, char *argv[]) {
  // A call to pystub() to force libpystub.so to be linked in.
  pystub();

  DXFPoints prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
