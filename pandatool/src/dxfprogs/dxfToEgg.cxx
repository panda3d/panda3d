// Filename: dxfToEgg.cxx
// Created by:  drose (04May04)
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

#include "dxfToEgg.h"

#include "dxfToEggConverter.h"

////////////////////////////////////////////////////////////////////
//     Function: DXFToEgg::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DXFToEgg::
DXFToEgg() :
  SomethingToEgg("DXF", ".dxf")
{
  add_units_options();
  add_normals_options();
  add_transform_options();

  set_program_description
    ("This program converts DXF (AutoCAD interchange format) to egg.  It "
     "only converts polygon data, with no fancy tricks.  DXF does not support "
     "hierarchical databases, so dxf2egg creates a single group at the root "
     "level for each layer in the DXF file.");

  redescribe_option
    ("cs",
     "Specify the coordinate system of the input " + _format_name +
     " file.  Normally, this is z-up.");

  _coordinate_system = CS_zup_right;
}

////////////////////////////////////////////////////////////////////
//     Function: DXFToEgg::run
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void DXFToEgg::
run() {
  nout << "Reading " << _input_filename << "\n";

  _data->set_coordinate_system(_coordinate_system);

  DXFToEggConverter converter;
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
  DXFToEgg prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
