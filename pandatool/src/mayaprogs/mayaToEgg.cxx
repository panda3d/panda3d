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

////////////////////////////////////////////////////////////////////
//     Function: MayaToEgg::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
MayaToEgg::
MayaToEgg() :
  SomethingToEgg("Maya", ".mb")
{
  add_animation_options();
  add_units_options();
  add_normals_options();
  add_transform_options();
  add_texture_path_options();
  add_rel_dir_options();
  add_search_path_options(false);

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
    mayaegg_cat->set_severity(NS_spam);
  } else if (_verbose >= 2) {
    mayaegg_cat->set_severity(NS_debug);
  } else if (_verbose >= 1) {
    mayaegg_cat->set_severity(NS_info);
  }

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

  // Copy in the animation parameters.
  converter.set_animation_convert(_animation_convert);
  converter.set_character_name(_character_name);
  if (_got_start_frame) {
    converter.set_start_frame(_start_frame);
  }
  if (_got_end_frame) {
    converter.set_end_frame(_end_frame);
  }
  if (_got_frame_inc) {
    converter.set_frame_inc(_frame_inc);
  }
  if (_got_input_frame_rate) {
    converter.set_input_frame_rate(_input_frame_rate);
  }
  if (_got_output_frame_rate) {
    converter.set_output_frame_rate(_output_frame_rate);
  }

  // Set the coordinate system to match Maya's.
  if (!_got_coordinate_system) {
    _coordinate_system = converter._maya->get_coordinate_system();
  }
  _data.set_coordinate_system(_coordinate_system);

  // Get the units from the Maya file, if the user didn't override.
  if (_input_units == DU_invalid) {
    _input_units = converter._maya->get_units();
  }

  converter.set_egg_data(&_data, false);
  converter.set_texture_path_convert(_texture_path_convert, _make_rel_dir);

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
