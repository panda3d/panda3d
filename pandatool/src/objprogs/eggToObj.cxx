// Filename: eggToObj.cxx
// Created by:  drose (28Feb12)
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

#include "eggToObj.h"
#include "pystub.h"
#include "eggPolygon.h"
#include "eggGroupNode.h"
#include "dcast.h"
#include "string_utils.h"

////////////////////////////////////////////////////////////////////
//     Function: EggToObj::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggToObj::
EggToObj() :
  EggToSomething("Obj", ".obj", true, false)
{
  set_program_brief("convert .egg files to .obj");
  set_program_description
    ("This program converts egg files to obj.  It "
     "only converts polygon data, with no fancy tricks.  "
     "Very bare-bones at the moment; not even texture maps are supported.");

  redescribe_option
    ("cs",
     "Specify the coordinate system of the resulting " + _format_name +
     " file.  Normally, this is z-up.");

  add_option
    ("C", "", 0,
     "Clean out higher-order polygons by subdividing into triangles.",
     &EggToObj::dispatch_none, &_triangulate_polygons);

  _coordinate_system = CS_zup_right;
  _got_coordinate_system = true;
}

////////////////////////////////////////////////////////////////////
//     Function: EggToObj::run
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void EggToObj::
run() {
  if (_triangulate_polygons) {
    nout << "Triangulating polygons.\n";
    int num_produced = _data->triangulate_polygons(~0);
    nout << "  (" << num_produced << " triangles produced.)\n";
  }

  EggToObjConverter saver;
  saver.set_egg_data(_data);

  if (!saver.write_file(get_output_filename())) {
    nout << "An error occurred while writing.\n";
    exit(1);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggToObj::handle_args
//       Access: Protected, Virtual
//  Description: Does something with the additional arguments on the
//               command line (after all the -options have been
//               parsed).  Returns true if the arguments are good,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool EggToObj::
handle_args(ProgramBase::Args &args) {
  return EggToSomething::handle_args(args);
}

int main(int argc, char *argv[]) {
  // A call to pystub() to force libpystub.so to be linked in.
  pystub();

  EggToObj prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
