// Filename: somethingToEgg.cxx
// Created by:  drose (15Feb00)
// 
////////////////////////////////////////////////////////////////////

#include "somethingToEgg.h"

////////////////////////////////////////////////////////////////////
//     Function: SomethingToEgg::Constructor
//       Access: Public
//  Description: The first parameter to the constructor should be the
//               one-word name of the file format that is to be read,
//               for instance "OpenFlight" or "Alias".  It's just used
//               in printing error messages and such.
////////////////////////////////////////////////////////////////////
SomethingToEgg::
SomethingToEgg(const string &format_name, 
	       const string &preferred_extension,
	       bool allow_last_param, bool allow_stdout) : 
  EggConverter(format_name, preferred_extension, allow_last_param, allow_stdout)
{
  clear_runlines();
  if (_allow_last_param) {
    add_runline("[opts] input" + _preferred_extension + " output.egg");
  }
  add_runline("[opts] -o output.egg input" + _preferred_extension);
  if (_allow_stdout) {
    add_runline("[opts] input" + _preferred_extension + " >output.egg");
  }

  // -f doesn't make sense if we aren't reading egg files.
  remove_option("f");

  redescribe_option
    ("cs",
     "Specify the coordinate system of the input " + _format_name +
     " file.  Normally, this can inferred from the file itself.");

  add_option
    ("ui", "units", 40, 
     "Specify the units of the input " + _format_name +
     " file.  Normally, this can be inferred from the file itself.",
     &SomethingToEgg::dispatch_units, NULL, &_input_units);

  add_option
    ("uo", "units", 40, 
     "Specify the units of the resulting egg file.  If this is "
     "specified, the vertices in the egg file will be scaled as "
     "necessary to make the appropriate units conversion; otherwise, "
     "the vertices in the input file will be converted exactly as they are.",
     &SomethingToEgg::dispatch_units, NULL, &_output_units);

  _input_units = DU_invalid;
  _output_units = DU_invalid;

  add_transform_options();
}

////////////////////////////////////////////////////////////////////
//     Function: SomethingToEgg::apply_units_scale
//       Access: Protected
//  Description: Applies the scale indicated by the input and output
//               units to the indicated egg file.  This is normally
//               done automatically when the file is written out.
////////////////////////////////////////////////////////////////////
void SomethingToEgg::
apply_units_scale(EggData &data) {
  if (_output_units != DU_invalid && _input_units != DU_invalid &&
      _input_units != _output_units) {
    nout << "Converting from " << format_long_unit(_input_units)
	 << " to " << format_long_unit(_output_units) << "\n";
    double scale = convert_units(_input_units, _output_units);
    data.transform(LMatrix4d::scale_mat(scale));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SomethingToEgg::handle_args
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
bool SomethingToEgg::
handle_args(Args &args) {
  if (_allow_last_param && !_got_output_filename && args.size() > 1) {
    _got_output_filename = true;
    _output_filename = args.back();
    args.pop_back();

    if (!(_output_filename.get_extension() == "egg")) {
      nout << "Output filename " << _output_filename 
	   << " does not end in .egg.  If this is really what you intended, "
	"use the -o output_file syntax.\n";
      return false;
    }

    if (!verify_output_file_safe()) {
      return false;
    }
  }

  if (args.empty()) {
    nout << "You must specify the " << _format_name 
	  << " file to read on the command line.\n";
    return false;
  }

  if (args.size() != 1) {
    nout << "You may only specify one " << _format_name 
	 << " file to read on the command line.  "
	 << "You specified: ";
    copy(args.begin(), args.end(), ostream_iterator<string>(nout, " "));
    nout << "\n";
    return false;
  }

  _input_filename = args[0];

  if (!_input_filename.exists()) {
    nout << "Cannot find input file " << _input_filename << "\n";
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: SomethingToEgg::post_process_egg_file
//       Access: Protected, Virtual
//  Description: Performs any processing of the egg file that is
//               appropriate before writing it out.  This includes any
//               normal adjustments the user requested via -np, etc.
//
//               Normally, you should not need to call this function
//               directly; write_egg_file() calls it for you.  You
//               should call this only if you do not use
//               write_egg_file() to write out the resulting egg file.
////////////////////////////////////////////////////////////////////
void SomethingToEgg::
post_process_egg_file() {
  apply_units_scale(_data);
  EggConverter::post_process_egg_file();
}
