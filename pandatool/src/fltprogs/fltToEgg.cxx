// Filename: fltToEgg.cxx
// Created by:  drose (17Apr01)
// 
////////////////////////////////////////////////////////////////////

#include "fltToEgg.h"

#include <fltToEggConverter.h>
#include <config_flt.h>

////////////////////////////////////////////////////////////////////
//     Function: FltToEgg::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
FltToEgg::
FltToEgg() :
  SomethingToEgg("MultiGen", ".flt")
{
  add_units_options();
  add_normals_options();
  add_transform_options();
  add_texture_path_options();
  add_model_path_options();
  add_rel_dir_options();
  add_search_path_options();

  set_program_description
    ("This program converts MultiGen OpenFlight (.flt) files to egg.  Most "
     "features of MultiGen that are also recognized by egg are supported.");

  redescribe_option
    ("cs",
     "Specify the coordinate system of the input " + _format_name +
     " file.  Normally, this is z-up.");

  _coordinate_system = CS_zup_right;
}

////////////////////////////////////////////////////////////////////
//     Function: FltToEgg::run
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void FltToEgg::
run() {
  PT(FltHeader) header = new FltHeader;
  header->set_texture_path(_search_path);
  header->set_model_path(_search_path);

  nout << "Reading " << _input_filename << "\n";
  FltError result = header->read_flt(_input_filename);
  if (result != FE_ok) {
    nout << "Unable to read: " << result << "\n";
    exit(1);
  }

  header->check_version();

  _data.set_coordinate_system(_coordinate_system);

  if (_input_units == DU_invalid) {
    _input_units = header->get_units();
  }

  FltToEggConverter converter;
  converter.set_egg_data(&_data, false);
  converter.set_texture_path_convert(_texture_path_convert, _make_rel_dir);
  converter.set_model_path_convert(_model_path_convert, _make_rel_dir);

  if (!converter.convert_flt(header)) {
    nout << "Errors in conversion.\n";
    exit(1);
  }

  write_egg_file();
  nout << "\n";
}


int main(int argc, char *argv[]) {
  init_libflt();
  FltToEgg prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
