// Filename: eggToSomething.cxx
// Created by:  drose (15Feb00)
// 
////////////////////////////////////////////////////////////////////

#include "eggToSomething.h"

////////////////////////////////////////////////////////////////////
//     Function: EggToSomething::Constructor
//       Access: Public
//  Description: The first parameter to the constructor should be the
//               one-word name of the file format that is to be read,
//               for instance "OpenFlight" or "Alias".  It's just used
//               in printing error messages and such.
////////////////////////////////////////////////////////////////////
EggToSomething::
EggToSomething(const string &format_name, 
	       const string &preferred_extension,
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

  string o_description;

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
}

////////////////////////////////////////////////////////////////////
//     Function: EggToSomething::handle_args
//       Access: Protected, Virtual
//  Description: Does something with the additional arguments on the
//               command line (after all the -options have been
//               parsed).  Returns true if the arguments are good,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool EggToSomething::
handle_args(ProgramBase::Args &args) {
  if (!check_last_arg(args, 1)) {
    return false;
  }

  return EggConverter::handle_args(args);
}
