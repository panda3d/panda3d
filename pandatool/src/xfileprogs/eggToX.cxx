// Filename: eggToX.cxx
// Created by:  drose (19Jun01)
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

#include "eggToX.h"
#include "config_xfile.h"

////////////////////////////////////////////////////////////////////
//     Function: EggToX::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggToX::
EggToX() : EggToSomething("DirectX", ".x", true, false) {
  add_texture_options();
  add_delod_options(0.0);

  set_program_description
    ("This program reads an Egg file and outputs an equivalent, "
     "or nearly equivalent, DirectX-style .x file.  Only simple "
     "hierarchy and polygon meshes are supported; advanced features "
     "like LOD's, decals, and characters cannot be supported.");

  add_option
    ("m", "", 0,
     "Convert all the objects in the egg file as one big mesh, instead of "
     "preserving the normal egg hierarchy.",
     &EggToX::dispatch_none, &xfile_one_mesh);

  // X files are always y-up-left.
  remove_option("cs");
  _got_coordinate_system = true;
  _coordinate_system = CS_yup_left;

  // We always have -f on: force complete load.  X files don't support
  // external references.
  remove_option("f");
  _force_complete = true;
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

  if (!do_reader_options()) {
    exit(1);
  }

  if (!_x.add_tree(_data)) {
    nout << "Unable to define egg structure.\n";
    exit(1);
  }

  _x.close();
}


int main(int argc, char *argv[]) {
  init_libxfile();
  EggToX prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
