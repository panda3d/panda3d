// Filename: fltToEgg.cxx
// Created by:  drose (17Apr01)
// 
////////////////////////////////////////////////////////////////////

#include "fltToEgg.h"

#include <fltToEggConverter.h>

////////////////////////////////////////////////////////////////////
//     Function: FltToEgg::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
FltToEgg::
FltToEgg() :
  SomethingToEgg("MultiGen", ".flt")
{
  add_normals_options();

  set_program_description
    ("This program converts MultiGen OpenFlight (.flt) files to egg.  "
     "Nothing fancy yet.");

  add_option
    ("tp", "path", 0, 
     "Add the indicated colon-delimited paths to the path that is searched "
     "for textures referenced by the flt file.  This "
     "option may also be repeated to add multiple paths.",
     &FltToEgg::dispatch_search_path, NULL, &_texture_path);

  redescribe_option
    ("cs",
     "Specify the coordinate system of the input " + _format_name +
     "file.  Normally, this is z-up.");

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
  header->set_texture_path(_texture_path);

  nout << "Reading " << _input_filename << "\n";
  FltError result = header->read_flt(_input_filename);
  if (result != FE_ok) {
    nout << "Unable to read: " << result << "\n";
    exit(1);
  }

  header->check_version();

  _data.set_coordinate_system(_coordinate_system);

  FltToEggConverter converter(_data);
  if (!converter.convert_flt(header)) {
    nout << "Errors in conversion.\n";
    exit(1);
  }

  write_egg_file();
}


int main(int argc, char *argv[]) {
  FltToEgg prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
