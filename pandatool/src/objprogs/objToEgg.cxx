// Filename: objToEgg.cxx
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

#include "objToEgg.h"

#include "objToEggConverter.h"
#include "pystub.h"

////////////////////////////////////////////////////////////////////
//     Function: ObjToEgg::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ObjToEgg::
ObjToEgg() :
  SomethingToEgg("obj", ".obj")
{
  add_units_options();
  add_points_options();
  add_normals_options();
  add_transform_options();

  set_program_brief("convert .obj files to .egg");
  set_program_description
    ("This program converts obj files to egg.  It "
     "only converts polygon data, with no fancy tricks.  "
     "Very bare-bones at the moment; not even texture maps are supported.");

  redescribe_option
    ("cs",
     "Specify the coordinate system of the input " + _format_name +
     " file.  Normally, this is z-up.");

  _coordinate_system = CS_zup_right;
}

////////////////////////////////////////////////////////////////////
//     Function: ObjToEgg::run
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void ObjToEgg::
run() {
  nout << "Reading " << _input_filename << "\n";

  _data->set_coordinate_system(_coordinate_system);

  ObjToEggConverter converter;
  converter.set_egg_data(_data);
  converter._allow_errors = _allow_errors;

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

  ObjToEgg prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
