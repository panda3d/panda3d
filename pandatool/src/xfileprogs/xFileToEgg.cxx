/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xFileToEgg.cxx
 * @author drose
 * @date 2001-06-21
 */

#include "xFileToEgg.h"
#include "xFileToEggConverter.h"
#include "config_xfile.h"

/**
 *
 */
XFileToEgg::
XFileToEgg() :
  SomethingToEgg("DirectX", ".x")
{
  add_path_replace_options();
  add_path_store_options();
  add_units_options();
  add_normals_options();
  add_transform_options();

  set_program_brief("convert a DirectX .x file to an .egg file");
  set_program_description
    ("This program converts DirectX retained-mode (.x) files to egg.  "
     "Polygon meshes, materials, and textures, as well as skeleton "
     "animation and skinning data, are supported.  All animations "
     "found in the source .x file are written together into the same "
     "egg file.");

  add_option
    ("a", "name", 0,
     "Specify the name of the animated character to generate.  This option "
     "forces the model to be converted as an animatable character, even "
     "if animation channels are not found in the file.  Without this "
     "option, the model is converted as a static model (which "
     "is usually more efficient to load within Panda), unless animation "
     "channels are present in the .x file.",
     &XFileToEgg::dispatch_string, &_make_char, &_char_name);

  add_option
    ("fr", "fps", 0,
     "Specify the frame rate of the resulting animation.  If this is "
     "omitted or 0, the frame rate is inferred from the file itself; but "
     "note that the file must contain evenly-spaced keyframes.",
     &XFileToEgg::dispatch_double, nullptr, &_frame_rate);

  add_option
    ("anim", "", 0,
     "Generate animation data only (all geometry will be discarded).",
     &XFileToEgg::dispatch_none, &_keep_animation);

  add_option
    ("model", "", 0,
     "Generate model data only (all animation data will be discarded).",
     &XFileToEgg::dispatch_none, &_keep_model);

  redescribe_option
    ("ui",
     "Specify the units of the input " + _format_name + " file.");

  redescribe_option
    ("uo",
     "Specify the units of the resulting egg file.  If both this and -ui are "
     "specified, the vertices in the egg file will be scaled as "
     "necessary to make the appropriate units conversion; otherwise, "
     "the vertices will be left as they are.");

  redescribe_option
    ("cs",
     "Specify the coordinate system of the input " + _format_name +
     " file.  Normally, this is y-up-left.");

  _frame_rate = 0.0;
  _coordinate_system = CS_yup_left;
}

/**
 *
 */
void XFileToEgg::
run() {
  _data->set_coordinate_system(_coordinate_system);

  XFileToEggConverter converter;
  converter.set_egg_data(_data);

  converter._frame_rate = _frame_rate;
  converter._make_char = _make_char;
  converter._char_name = _char_name;
  converter._keep_model = _keep_model;
  converter._keep_animation = _keep_animation;

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
