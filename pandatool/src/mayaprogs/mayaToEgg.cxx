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
#include "mayaToEggConverter.h"
#include "config_mayaegg.h"
#include "config_maya.h"  // for maya_cat

////////////////////////////////////////////////////////////////////
//     Function: MayaToEgg::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
MayaToEgg::
MayaToEgg() :
  SomethingToEgg("Maya", ".mb")
{
  add_path_replace_options();
  add_path_store_options();
  add_animation_options();
  add_units_options();
  add_normals_options();
  add_transform_options();

  set_program_description
    ("This program converts Maya model files to egg.  Nothing fancy yet.");

  add_option
    ("p", "", 0,
     "Generate polygon output only.  Tesselate all NURBS surfaces to "
     "polygons via the built-in Maya tesselator.  The tesselation will "
     "be based on the tolerance factor given by -ptol.",
     &MayaToEgg::dispatch_none, &_polygon_output);

  add_option
    ("ptol", "tolerance", 0,
     "Specify the fit tolerance for Maya polygon tesselation.  The smaller "
     "the number, the more polygons will be generated.  The default is "
     "0.01.",
     &MayaToEgg::dispatch_double, NULL, &_polygon_tolerance);

  add_option
    ("notrans", "", 0,
     "Don't convert explicit DAG transformations given in the Maya file.  "
     "Instead, convert all vertices to world space and write the file as "
     "one big transform space.  Using this option doesn't change the "
     "position of objects in the scene, just the number of explicit "
     "transforms appearing in the resulting egg file.",
     &MayaToEgg::dispatch_none, &_ignore_transforms);

  add_option
    ("v", "", 0,
     "Increase verbosity.  More v's means more verbose.",
     &MayaToEgg::dispatch_count, NULL, &_verbose);

  _polygon_tolerance = 0.01;
  _verbose = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEgg::run
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void MayaToEgg::
run() {
  // Set the verbose level by using Notify.
  if (_verbose >= 3) {
    maya_cat->set_severity(NS_spam);
    mayaegg_cat->set_severity(NS_spam);
  } else if (_verbose >= 2) {
    maya_cat->set_severity(NS_debug);
    mayaegg_cat->set_severity(NS_debug);
  } else if (_verbose >= 1) {
    maya_cat->set_severity(NS_info);
    mayaegg_cat->set_severity(NS_info);
  }

  // Let's open the output file before we initialize Maya, since Maya
  // now has a nasty habit of changing the current directory.
  get_output();

  nout << "Initializing Maya.\n";
  MayaToEggConverter converter(_program_name);
  if (!converter.open_api()) {
    nout << "Unable to initialize Maya.\n";
    exit(1);
  }

  // Copy in the command-line parameters.
  converter._polygon_output = _polygon_output;
  converter._polygon_tolerance = _polygon_tolerance;
  converter._ignore_transforms = _ignore_transforms;

  // Copy in the path and animation parameters.
  apply_parameters(converter);

  // Set the coordinate system to match Maya's.
  if (!_got_coordinate_system) {
    _coordinate_system = converter._maya->get_coordinate_system();
  }
  _data.set_coordinate_system(_coordinate_system);

  // Use the standard Maya units, if the user didn't specify
  // otherwise.  This always returns centimeters, which is the way all
  // Maya files are stored internally (and is the units returned by
  // all of the API functions called here).
  if (_input_units == DU_invalid) {
    _input_units = converter._maya->get_units();
  }

  converter.set_egg_data(&_data, false);
  apply_parameters(converter);

  if (!converter.convert_file(_input_filename)) {
    nout << "Errors in conversion.\n";
    exit(1);
  }

  write_egg_file();
  nout << "\n";
}


int main(int argc, char *argv[]) {
  MayaToEgg prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
