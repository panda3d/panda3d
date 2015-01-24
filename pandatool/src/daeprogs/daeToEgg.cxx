// Filename: daeToEgg.cxx
// Created by:  pro-rsoft (08May08)
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

#include "daeToEgg.h"

#include "daeToEggConverter.h"
#include "pystub.h"

////////////////////////////////////////////////////////////////////
//     Function: DAEToEgg::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DAEToEgg::
DAEToEgg():
  SomethingToEgg("COLLADA", ".dae")
{
  add_units_options();
  add_normals_options();
  add_transform_options();

  add_option
    ("invtrans", "", false,
     "Import the .dae file using inverted transparency. "
     "This is useful when importing COLLADA files from some authoring tools "
     "that export models with inverted transparency, such as Google SketchUp.",
     &SomethingToEgg::dispatch_none, &_invert_transparency);

  set_program_brief("convert COLLADA assets into .egg files");
  set_program_description
    ("This program converts .dae files (COLLADA Digital Asset Exchange) to .egg.");

  _coordinate_system = CS_yup_right;
}

////////////////////////////////////////////////////////////////////
//     Function: DAEToEgg::run
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void DAEToEgg::
run() {
  nout << "Reading " << _input_filename << "\n";

  _data->set_coordinate_system(_coordinate_system);

  DAEToEggConverter converter;
  converter.set_egg_data(_data);
  converter._allow_errors = _allow_errors;
  converter._invert_transparency = _invert_transparency;

  apply_parameters(converter);

  if (!converter.convert_file(_input_filename)) {
    nout << "Errors in conversion.\n";
    exit(1);
  }

  write_egg_file();
  nout << "\n";
}


int main(int argc, char *argv[]) {
  // A call to pystub() to force libpystub.so to be linked in.
  pystub();

  DAEToEgg prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
