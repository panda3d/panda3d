// Filename: mayaToEgg.cxx
// Created by:  drose (15Feb00)
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

#include "mayaToEgg.h"
#include "global_parameters.h"

////////////////////////////////////////////////////////////////////
//     Function: MayaToEgg::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
MayaToEgg::
MayaToEgg() :
  SomethingToEgg("Maya", ".mb")
{
  add_units_options();
  add_normals_options();
  add_transform_options();
  //  add_texture_path_options();
  //  add_rel_dir_options();
  //  add_search_path_options(false);

  set_program_description
    ("This program converts Maya model files to egg.  Nothing fancy yet.");

  add_option
    ("p", "", 0,
     "Generate polygon output only.  Tesselate all NURBS surfaces to "
     "polygons via the built-in Maya tesselator.  The tesselation will "
     "be based on the tolerance factor given by -ptol.",
     &MayaToEgg::dispatch_none, &polygon_output);

  add_option
    ("ptol", "tolerance", 0,
     "Specify the fit tolerance for Maya polygon tesselation.  The smaller "
     "the number, the more polygons will be generated.  The default is "
     "0.01.",
     &MayaToEgg::dispatch_double, NULL, &polygon_tolerance);

  add_option
    ("notrans", "", 0,
     "Don't convert explicit DAG transformations given in the Maya file.  "
     "Instead, convert all vertices to world space and write the file as "
     "one big transform space.  Using this option doesn't change the "
     "position of objects in the scene, just the number of explicit "
     "transforms appearing in the resulting egg file.",
     &MayaToEgg::dispatch_none, &ignore_transforms);

  add_option
    ("v", "", 0,
     "Increase verbosity.  More v's means more verbose.",
     &MayaToEgg::dispatch_count, NULL, &verbose);
  verbose = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEgg::run
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void MayaToEgg::
run() {
  nout << "Initializing Maya.\n";
  if (!_maya.init(_program_name)) {
    nout << "Unable to initialize Maya.\n";
    exit(1);
  }

  if (!_maya.read(_input_filename.c_str())) {
    nout << "Error reading " << _input_filename << ".\n";
    exit(1);
  }

  // Set the coordinate system to match Maya's.
  if (!_got_coordinate_system) {
    _coordinate_system = MayaFile::get_coordinate_system();
  }
  _data.set_coordinate_system(_coordinate_system);

  // Get the units from the Maya file, if the user didn't override.
  if (_input_units == DU_invalid) {
    _input_units = MayaFile::get_units();
  }

  _maya.make_egg(_data);

  write_egg_file();
}


int main(int argc, char *argv[]) {
  MayaToEgg prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
