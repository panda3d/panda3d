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
     "file.  Normally, this can inferred from the file itself.");
}

////////////////////////////////////////////////////////////////////
//     Function: SomethingToEgg::handle_args
//       Access: Public
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
