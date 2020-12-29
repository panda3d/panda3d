/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggToMaya.cxx
 * @author drose
 * @date 2005-08-11
 */

#include "eggToMaya.h"
#include "mayaEggLoader.h"
#include "mayaApi.h"
#include "mayaConversionServer.h"

// We must define this to prevent Maya from doubly-declaring its MApiVersion
// string in this file as well as in libmayaegg.
#define _MApiVersion

#include "pre_maya_include.h"
#include <maya/MString.h>
#include <maya/MFileIO.h>
#include "post_maya_include.h"

/**
 *
 */
EggToMaya::
EggToMaya() :
  EggToSomething("Maya", ".mb", true, false)
{
  add_units_options();

  set_binary_output(true);
  set_program_brief("convert .egg files to Maya .mb or .ma files");
  set_program_description
    ("egg2maya converts files from egg format to Maya .mb or .ma "
     "format.  It contains support for basic geometry (polygons with textures)."
     "It also supports animation for joints.");

  add_option
    ("a", "", 0,
     "Convert animation tables.",
     &EggToMaya::dispatch_none, &_convert_anim);

  add_option
    ("m", "", 0,
     "Convert polygon models.  You may specify both -a and -m at the same "
     "time.  If you specify neither, the default is -m.",
     &EggToMaya::dispatch_none, &_convert_model);

  add_option
    ("nv", "", 0,
     "respect vertex and polygon normals.",
     &EggToMaya::dispatch_none, &_respect_normals);

  add_option("server", "", 0,
    "Runs the Maya model conversion server. This server can be used in tandem "
    "with the egg2maya_client and maya2egg_client utilities to batch convert "
    "both Maya and Panda3D model files.",
    &EggToMaya::dispatch_none, &_run_server);

  // Maya files always store centimeters.
  _output_units = DU_centimeters;
}

/**
 * Attempts to create the global Maya API.
 * Exits the program if unsuccessful.
 */
PT(MayaApi) EggToMaya::
open_api() {
  if (!MayaApi::is_api_valid()) {
    nout << "Initializing Maya...\n";
  }

  PT(MayaApi) api = MayaApi::open_api(_program_name, true, true);

  if (!api || !api->is_valid()) {
    nout << "Unable to initialize Maya.\n";
    exit(1);
  }

  return api;
}

/**
 * Returns true if the model has been successfully converted.
 */
bool EggToMaya::
run() {
  if (!_convert_anim && !_convert_model) {
    _convert_model = true;
  }

  // Let's convert the output file to a full path before we initialize Maya,
  // since Maya now has a nasty habit of changing the current directory.
  _output_filename.make_absolute();

  PT(MayaApi) maya = open_api();

  MStatus status;
  status = MFileIO::newFile(true);
  if (!status) {
    status.perror("Could not initialize file");
    return false;
  }

  // [gjeon] since maya's internal unit is fixed to cm and when we can't
  // change UI unit without affecting data all distance data is converted to
  // cm we need to convert them back to proper output unit user provided here
  // along with UI unit
  maya->set_units(_output_units);

  if (_output_units != DU_centimeters && _output_units != DU_invalid) {
    nout << "Converting from centimeters"
         << " to " << format_long_unit(_output_units) << "\n";
  }

  // Now convert the data.
  if (!MayaLoadEggData(_data, true, _convert_model, _convert_anim, _respect_normals)) {
    nout << "Unable to convert egg file.\n";
    return false;
  }

  if (!maya->write(_output_filename)) {
    status.perror("Could not save file");
    return false;
  }

  /*
  // And write out the resulting Maya file.
  string os_specific = _output_filename.to_os_generic();
  const char *file_type = NULL;
  if (_output_filename.get_extension() == "mb") {
    file_type = "mayaBinary";
  }
  status = MFileIO::saveAs(os_specific.c_str(), file_type);
  if (!status) {
    status.perror("Could not save file");
    return false;
  }
  */

  return true;
}

/**
 * Processes the arguments parsed by the program.
 *
 * If the server flag is specified, the Maya conversion server is started
 * up rather than the usual conversion utility functionality.
 */
bool EggToMaya::
handle_args(ProgramBase::Args &args) {
  if (_run_server) {
    open_api();

    MayaConversionServer server;
    server.listen();
    exit(0);
    return true;
  }

  return EggToSomething::handle_args(args);
}
