// Filename: lwoToEgg.cxx
// Created by:  drose (17Apr01)
// 
////////////////////////////////////////////////////////////////////

#include "lwoToEgg.h"

#include <lwoToEggConverter.h>
#include <lwoHeader.h>
#include <lwoInputFile.h>
#include <config_lwo.h>

////////////////////////////////////////////////////////////////////
//     Function: LwoToEgg::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
LwoToEgg::
LwoToEgg() :
  SomethingToEgg("Lightwave", ".lwo")
{
  add_units_options();
  add_normals_options();
  add_transform_options();
  add_texture_path_options();
  add_rel_dir_options();
  add_search_path_options(true);

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
  _data.set_coordinate_system(_coordinate_system);

  if (_input_units == DU_invalid) {
    _input_units = DU_meters;
  }

  LwoToEggConverter converter;
  converter.set_egg_data(&_data, false);
  converter.set_texture_path_convert(_texture_path_convert, _make_rel_dir);
  converter.set_model_path_convert(_model_path_convert, _make_rel_dir);

  if (!converter.convert_file(_input_filename)) {
    nout << "Errors in conversion.\n";
    exit(1);
  }

  write_egg_file();
  nout << "\n";
}


int main(int argc, char *argv[]) {
  init_liblwo();
  LwoToEgg prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
