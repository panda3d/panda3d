// Filename: vrmlToEgg.cxx
// Created by:  drose (01Oct04)
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

#include "vrmlToEgg.h"

#include "vrmlToEggConverter.h"
#include "pystub.h"

////////////////////////////////////////////////////////////////////
//     Function: VRMLToEgg::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
VRMLToEgg::
VRMLToEgg() :
  SomethingToEgg("VRML", ".wrl")
{
  add_units_options();
  add_normals_options();
  add_transform_options();

  set_program_brief("convert VRML 2.0 model files to .egg");
  set_program_description
    ("This program converts VRML 2.0 model files to egg.  Animated files, "
     "and VRML 1.0 files, are not supported.");

  redescribe_option
    ("cs",
     "Specify the coordinate system of the input " + _format_name +
     " file.  Normally, this is y-up.");

  _coordinate_system = CS_yup_right;
}

////////////////////////////////////////////////////////////////////
//     Function: VRMLToEgg::run
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void VRMLToEgg::
run() {
  nout << "Reading " << _input_filename << "\n";

  _data->set_coordinate_system(_coordinate_system);

  VRMLToEggConverter converter;
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

  VRMLToEgg prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
