// Filename: eggToX.cxx
// Created by:  drose (19Jun01)
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

#include "eggToX.h"

////////////////////////////////////////////////////////////////////
//     Function: EggToX::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggToX::
EggToX() : EggToSomething("DirectX", "x") {
  set_program_description
    ("This program reads an Egg file and outputs an equivalent, "
     "or nearly equivalent, DirectX-style .x file.  Only simple "
     "hierarchy and polygon meshes are supported; advanced features "
     "like LOD's, decals, and characters cannot be supported.");

  // X files are always y-up-left.
  remove_option("cs");
  _got_coordinate_system = true;
  _coordinate_system = CS_yup_left;
}


////////////////////////////////////////////////////////////////////
//     Function: EggToX::run
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void EggToX::
run() {
  if (!_x.open(get_output_filename())) {
    nout << "Unable to open " << get_output_filename() << " for output.\n";
    exit(1);
  }

  if (!_x.add_tree(_data)) {
    nout << "Unable to define egg structure.\n";
    exit(1);
  }

  _x.close();
}


int main(int argc, char *argv[]) {
  EggToX prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
