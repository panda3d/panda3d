// Filename: eggToX.cxx
// Created by:  drose (19Jun01)
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

#include "eggToX.h"
#include "config_xfile.h"
#include "pystub.h"

////////////////////////////////////////////////////////////////////
//     Function: EggToX::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggToX::
EggToX() : EggToSomething("DirectX", ".x", true, false) {
  add_texture_options();
  add_delod_options(0.0);

  set_program_brief("convert an .egg file into a DirectX .x file");
  set_program_description
    ("This program reads an Egg file and outputs an equivalent, "
     "or nearly equivalent, DirectX-style .x file.  Only simple "
     "hierarchy and polygon meshes are supported; advanced features "
     "like LOD's, decals, and animation or skinning are not supported.");

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
  if (!do_reader_options()) {
    exit(1);
  }

  if (!_x.add_tree(_data)) {
    nout << "Unable to define egg structure.\n";
    exit(1);
  }

  if (!_x.write(get_output_filename())) {
    nout << "Unable to write " << get_output_filename() << ".\n";
    exit(1);
  }
}


int main(int argc, char *argv[]) {
  // A call to pystub() to force libpystub.so to be linked in.
  pystub();

  init_libxfile();
  EggToX prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
