// Filename: lwoToEgg.cxx
// Created by:  drose (17Apr01)
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

#include "lwoToEgg.h"

#include "lwoToEggConverter.h"
#include "lwoHeader.h"
#include "lwoInputFile.h"
#include "config_lwo.h"
#include "pystub.h"

////////////////////////////////////////////////////////////////////
//     Function: LwoToEgg::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
LwoToEgg::
LwoToEgg() :
  SomethingToEgg("Lightwave", ".lwo")
{
  add_path_replace_options();
  add_path_store_options();
  add_units_options();
  add_normals_options();
  add_transform_options();

  set_program_brief("convert a Lightwave Object file to .egg");
  set_program_description
    ("This program converts Lightwave Object (.lwo) files to egg.  Many "
     "rendering characteristics of Lightwave (like layered shaders, etc.) "
     "are not supported, but fundamental things like polygons and texture "
     "maps are.  This program is primarily designed to support files written "
     "by Lightwave version 6.x (LWO2 files), but it also has some limited "
     "support for version 5.x files (LWOB files).");

  redescribe_option
    ("cs",
     "Specify the coordinate system of the input " + _format_name +
     " file.  Normally, this is y-up-left.");

  redescribe_option
    ("ui",
     "Specify the units of the input Lightwave file.  By convention, "
     "this is assumed to be meters if it is unspecified.");

  _coordinate_system = CS_yup_left;
}

////////////////////////////////////////////////////////////////////
//     Function: LwoToEgg::run
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void LwoToEgg::
run() {
  _data->set_coordinate_system(_coordinate_system);

  if (_input_units == DU_invalid) {
    _input_units = DU_meters;
  }

  LwoToEggConverter converter;
  converter.set_egg_data(_data);
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

  init_liblwo();
  LwoToEgg prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
