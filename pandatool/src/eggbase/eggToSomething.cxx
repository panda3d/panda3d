/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggToSomething.cxx
 * @author drose
 * @date 2000-02-15
 */

#include "eggToSomething.h"

/**
 * The first parameter to the constructor should be the one-word name of the
 * file format that is to be read, for instance "OpenFlight" or "Alias".  It's
 * just used in printing error messages and such.
 */
EggToSomething::
EggToSomething(const std::string &format_name,
               const std::string &preferred_extension,
               bool allow_last_param, bool allow_stdout) :
  EggConverter(format_name, preferred_extension, allow_last_param,
               allow_stdout)
{
  clear_runlines();
  if (_allow_last_param) {
    add_runline("[opts] input.egg output" + _preferred_extension);
  }
  add_runline("[opts] -o output" + _preferred_extension + " input.egg");
  if (_allow_stdout) {
    add_runline("[opts] input.egg >output" + _preferred_extension);
  }

  std::string o_description;

  if (_allow_stdout) {
    if (_allow_last_param) {
      o_description =
        "Specify the filename to which the resulting " + format_name +
        " file will be written.  "
        "If this option is omitted, the last parameter name is taken to be the "
        "name of the output file, or standard output is used if there are no "
        "other parameters.";
    } else {
      o_description =
        "Specify the filename to which the resulting " + format_name +
        " file will be written.  "
        "If this option is omitted, the " + format_name +
        " file is written to standard output.";
    }
  } else {
    if (_allow_last_param) {
      o_description =
        "Specify the filename to which the resulting " + format_name +
        " file will be written.  "
        "If this option is omitted, the last parameter name is taken to be the "
        "name of the output file.";
    } else {
      o_description =
        "Specify the filename to which the resulting " + format_name +
        " file will be written.";
    }
  }

  redescribe_option("o", o_description);

  redescribe_option
    ("cs",
     "Specify the coordinate system of the resulting " + _format_name +
     " file.  This may be "
     "one of 'y-up', 'z-up', 'y-up-left', or 'z-up-left'.  The default "
     "is the same coordinate system as the input egg file.  If this is "
     "different from the input egg file, a conversion will be performed.");

  _input_units = DU_invalid;
  _output_units = DU_invalid;
}

/**
 * Adds -ui and -uo as valid options for this program.  If the user specifies
 * -uo and -ui, or just -uo and the program specifies -ui by setting
 * _input_units, the indicated units conversion will be automatically applied
 * before writing out the egg file.
 */
void EggToSomething::
add_units_options() {
  add_option
    ("ui", "units", 40,
     "Specify the units of the input egg file.  If this is "
     "specified, the vertices in the egg file will be scaled as "
     "necessary to make the appropriate units conversion; otherwise, "
     "the vertices will be left as they are.",
     &EggToSomething::dispatch_units, nullptr, &_input_units);

  add_option
    ("uo", "units", 40,
     "Specify the units of the resulting " + _format_name +
     " file.  Normally, the default units for the format are used.",
     &EggToSomething::dispatch_units, nullptr, &_output_units);
}

/**
 * Applies the scale indicated by the input and output units to the indicated
 * egg file.  This is normally done automatically when the file is read in.
 */
void EggToSomething::
apply_units_scale(EggData *data) {

  // [gjeon] since maya's internal unit is fixed to cm and when we can't
  // change UI unit without affecting data we need to convert data to cm for
  // now this will be set later to proper output unit user provided by using
  // MayaApi::set_units() in eggToMaya.cxx
  DistanceUnit output_units = _output_units;
  if (_format_name == "Maya")
    _output_units = DU_centimeters;

  if (_output_units != DU_invalid && _input_units != DU_invalid &&
      _input_units != _output_units) {
    nout << "Converting from " << format_long_unit(_input_units)
         << " to " << format_long_unit(_output_units) << "\n";
    double scale = convert_units(_input_units, _output_units);
    data->transform(LMatrix4d::scale_mat(scale));
  }
  _output_units = output_units;
}

/**
 * Performs any processing of the egg file that is appropriate after reading
 * it in.
 *
 * Normally, you should not need to call this function directly; it is called
 * automatically at startup.
 */
void EggToSomething::
pre_process_egg_file() {
  apply_units_scale(_data);
  EggConverter::pre_process_egg_file();
}

/**
 * Does something with the additional arguments on the command line (after all
 * the -options have been parsed).  Returns true if the arguments are good,
 * false otherwise.
 */
bool EggToSomething::
handle_args(ProgramBase::Args &args) {
  if (!check_last_arg(args, 1)) {
    return false;
  }

  return EggConverter::handle_args(args);
}
