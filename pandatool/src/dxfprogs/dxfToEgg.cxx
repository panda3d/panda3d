/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxfToEgg.cxx
 * @author drose
 * @date 2004-05-04
 */

#include "dxfToEgg.h"

#include "dxfToEggConverter.h"

/**
 *
 */
DXFToEgg::
DXFToEgg() :
  SomethingToEgg("DXF", ".dxf")
{
  add_units_options();
  add_normals_options();
  add_transform_options();

  set_program_brief("convert AutoCAD .dxf files to .egg files");
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

/**
 *
 */
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
