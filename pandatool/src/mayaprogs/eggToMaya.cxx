// Filename: eggToMaya.cxx
// Created by:  drose (11Aug05)
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

#include "eggToMaya.h"
#include "mayaEggLoader.h"
#include "mayaApi.h"

// We must define this to prevent Maya from doubly-declaring its
// MApiVersion string in this file as well as in libmayaegg.
#define _MApiVersion

#include "pre_maya_include.h"
#include <maya/MString.h>
#include <maya/MFileIO.h>
#include "post_maya_include.h"

////////////////////////////////////////////////////////////////////
//     Function: EggToMaya::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
EggToMaya::
EggToMaya() :
  EggToSomething("Maya", ".mb", true, false)
{
  add_units_options();

  set_binary_output(true);
  set_program_description
    ("egg2maya converts files from egg format to Maya .mb or .ma "
     "format.  At the moment, it contains support for basic geometry "
     "(polygons with textures).");

  add_option
    ("a", "", 0,
     "Convert animation tables.",
     &EggToMaya::dispatch_none, &_convert_anim);

  add_option
    ("m", "", 0,
     "Convert polygon models.  You may specify both -a and -m at the same "
     "time.  If you specify neither, the default is -m.",
     &EggToMaya::dispatch_none, &_convert_model);

  // Maya files always store centimeters.
  _output_units = DU_centimeters;
}

////////////////////////////////////////////////////////////////////
//     Function: EggToMaya::run
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void EggToMaya::
run() {
  if (!_convert_anim && !_convert_model) {
    _convert_model = true;
  }

  // Let's convert the output file to a full path before we initialize
  // Maya, since Maya now has a nasty habit of changing the current
  // directory.
  _output_filename.make_absolute();

  nout << "Initializing Maya.\n";
  PT(MayaApi) maya = MayaApi::open_api(_program_name);
  if (!maya->is_valid()) {
    nout << "Unable to initialize Maya.\n";
    exit(1);
  }

  MStatus status;
  status = MFileIO::newFile(true);
  if (!status) {
    status.perror("Could not initialize file");
    exit(1);
  }

  // Now convert the data.
  if (!MayaLoadEggData(_data, true, _convert_model, _convert_anim)) {
    nout << "Unable to convert egg file.\n";
    exit(1);
  }

  // And write out the resulting Maya file.
  string os_specific = _output_filename.to_os_generic();
  const char *file_type = NULL;
  if (_output_filename.get_extension() == "mb") {
    file_type = "mayaBinary";
  }
  status = MFileIO::saveAs(os_specific.c_str(), file_type);
  if (!status) {
    status.perror("Could not save file");
    exit(1);
  }
}

int main(int argc, char *argv[]) {
  EggToMaya prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
