// Filename: lwoToEgg.cxx
// Created by:  drose (17Apr01)
// 
////////////////////////////////////////////////////////////////////

#include "lwoToEgg.h"

#include <lwoToEggConverter.h>
#include <lwoHeader.h>
#include <lwoInputFile.h>

////////////////////////////////////////////////////////////////////
//     Function: LwoToEgg::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
LwoToEgg::
LwoToEgg() :
  SomethingToEgg("MultiGen", ".lwo")
{
  add_normals_options();
  add_transform_options();

  set_program_description
    ("This program converts Lightwave Object (.lwo) files to egg.  Many "
     "rendering characteristics of Lightwave (like layered shaders, etc.) "
     "are not supported, but fundamental things like polygons and texture "
     "maps are.");

  redescribe_option
    ("cs",
     "Specify the coordinate system of the input " + _format_name +
     " file.  Normally, this is y-up-left.");

  _coordinate_system = CS_yup_left;
}

////////////////////////////////////////////////////////////////////
//     Function: LwoToEgg::run
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void LwoToEgg::
run() {
  LwoInputFile in;

  nout << "Reading " << _input_filename << "\n";
  if (!in.open_read(_input_filename)) {
    nout << "Unable to open " << _input_filename << "\n";
    exit(1);
  }

  PT(IffChunk) chunk = in.get_chunk();
  if (chunk == (IffChunk *)NULL) {
    nout << "Unable to read " << _input_filename << "\n";
    exit(1);
  }

  if (!chunk->is_of_type(LwoHeader::get_class_type())) {
    nout << "File " << _input_filename << " is not a Lightwave Object file.\n";
    exit(1);
  }

  LwoHeader *header = DCAST(LwoHeader, chunk);

  _data.set_coordinate_system(_coordinate_system);

  if (_input_units == DU_invalid) {
    _input_units = DU_meters;
  }

  LwoToEggConverter converter(_data);

  if (!converter.convert_lwo(header)) {
    nout << "Errors in conversion.\n";
    exit(1);
  }

  write_egg_file();
  nout << "\n";
}


int main(int argc, char *argv[]) {
  LwoToEgg prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
