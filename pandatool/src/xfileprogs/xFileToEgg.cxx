// Filename: xFileToEgg.cxx
// Created by:  drose (21Jun01)
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

#include "xFileToEgg.h"
#include "xFileToEggConverter.h"
#include "config_xfile.h"

////////////////////////////////////////////////////////////////////
//     Function: XFileToEgg::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
XFileToEgg::
XFileToEgg() :
  SomethingToEgg("DirectX", ".x")
{
  add_path_replace_options();
  add_path_store_options();
  add_units_options();
  add_normals_options();
  add_transform_options();

  set_program_description
    ("This program converts DirectX retained-mode (.x) files to egg.  "
     "Polygon meshes, materials, and textures, as well as skeleton "
     "animation and skinning data, are supported.  All animations "
     "found in the source .x file are written together into the same "
     "egg file.");

  add_option
    ("a", "name", 0,
     "Convert as an animatable model, converting Frames into Joints.  "
     "Note that animation support in x2egg is currently experimental and "
     "likely to be broken.",
  /*
     "Convert as an animatable model, converting Frames into Joints.  This "
     "should be specified for a model which is intended to be animated.  The "
     "default is to convert the model as a normal static model, which is "
     "usually more optimal if animation is not required.",
  */
     &XFileToEgg::dispatch_string, &_make_char, &_char_name);

  redescribe_option
    ("cs",
     "Specify the coordinate system of the input " + _format_name +
     " file.  Normally, this is y-up-left.");

  _coordinate_system = CS_yup_left;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileToEgg::run
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void XFileToEgg::
run() {
  _data.set_coordinate_system(_coordinate_system);

  XFileToEggConverter converter;
  converter.set_egg_data(&_data, false);

  converter._make_char = _make_char;
  converter._char_name = _char_name;

  // Copy in the path and animation parameters.
  apply_parameters(converter);

  if (!converter.convert_file(_input_filename)) {
    nout << "Unable to read " << _input_filename << "\n";
    exit(1);
  }

  write_egg_file();
  nout << "\n";
}


int main(int argc, char *argv[]) {
  init_libxfile();
  XFileToEgg prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
