// Filename: eggWriter.cxx
// Created by:  drose (14Feb00)
// 
////////////////////////////////////////////////////////////////////

#include "eggWriter.h"

////////////////////////////////////////////////////////////////////
//     Function: EggWriter::Constructor
//       Access: Public
//  Description: Egg-writing type programs may specify their output
//               file using either the last-filename convention, the
//               -o convention, and/or implicitly writing the result
//               to standard output.  Not all interfaces are
//               appropriate for all applications; some may be
//               confusing or dangerous.
//
//               The calling application should pass allow_last_param
//               true to allow the user to specify the output filename
//               as the last parameter on the command line (the most
//               dangerous, but convenient, method), and allow_stdout
//               true to allow the user to omit the output filename
//               altogether and have the output implicitly go to
//               standard output (not terribly dangerous, but
//               inappropriate when writing binary file formats).
////////////////////////////////////////////////////////////////////
EggWriter::
EggWriter(bool allow_last_param, bool allow_stdout) {
  _allow_last_param = allow_last_param;
  _allow_stdout = allow_stdout;

  clear_runlines();
  if (_allow_last_param) {
    add_runline("[opts] output.egg");
  }
  add_runline("[opts] -o output.egg");
  if (_allow_stdout) {
    add_runline("[opts] >output.egg");
  }

  string o_description;

  if (_allow_stdout) {
    if (_allow_last_param) {
      o_description =
	"Specify the filename to which the resulting egg file will be written.  "
	"If this option is omitted, the last parameter name is taken to be the "
	"name of the output file, or standard output is used if there are no "
	"other parameters.";
    } else {
      o_description =
	"Specify the filename to which the resulting egg file will be written.  "
	"If this option is omitted, the egg file is written to standard output.";
    }
  } else {
    if (_allow_last_param) {
      o_description =
	"Specify the filename to which the resulting egg file will be written.  "
	"If this option is omitted, the last parameter name is taken to be the "
	"name of the output file.";
    } else {
      o_description =
	"Specify the filename to which the resulting egg file will be written.";
    }
  }

  add_option
    ("o", "filename", 50, o_description,
     &EggWriter::dispatch_filename, &_got_output_filename, &_output_filename);

  redescribe_option
    ("cs",
     "Specify the coordinate system of the resulting egg file.  This may be "
     "one of 'y-up', 'z-up', 'y-up-left', or 'z-up-left'.  The default is "
     "y-up.");

  _output_ptr = (ostream *)NULL;
}


////////////////////////////////////////////////////////////////////
//     Function: EggWriter::get_output
//       Access: Public
//  Description: Returns an output stream that corresponds to the
//               user's intended egg file output--either stdout, or
//               the named output file.
////////////////////////////////////////////////////////////////////
ostream &EggWriter::
get_output() {
  if (_output_ptr == (ostream *)NULL) {
    if (!_got_output_filename) {
      // No filename given; use standard output.
      assert(_allow_stdout);
      _output_ptr = &cout;
      
    } else {
      // Attempt to open the named file.
      unlink(_output_filename.c_str());
      _output_filename.make_dir();
      if (!_output_filename.open_write(_output_stream)) {
	nout << "Unable to write to " << _output_filename << "\n";
	exit(1);
      }
      nout << "Writing " << _output_filename << "\n";
      _output_ptr = &_output_stream;
    }
  }
  return *_output_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: EggWriter::has_output_filename
//       Access: Public
//  Description: Returns true if the user specified an output
//               filename, false otherwise (e.g. the output file is
//               implicitly stdout).
////////////////////////////////////////////////////////////////////
bool EggWriter::
has_output_filename() const {
  return _got_output_filename;
}

////////////////////////////////////////////////////////////////////
//     Function: EggWriter::get_output_filename
//       Access: Public
//  Description: If has_output_filename() returns true, this is the
//               filename that the user specified.  Otherwise, it
//               returns the empty string.
////////////////////////////////////////////////////////////////////
Filename EggWriter::
get_output_filename() const {
  if (_got_output_filename) {
    return _output_filename;
  }
  return Filename();
}

////////////////////////////////////////////////////////////////////
//     Function: EggWriter::handle_args
//       Access: Protected, Virtual
//  Description: Does something with the additional arguments on the
//               command line (after all the -options have been
//               parsed).  Returns true if the arguments are good,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool EggWriter::
handle_args(ProgramBase::Args &args) {
  if (_allow_last_param && !_got_output_filename && !args.empty()) {
    _got_output_filename = true;
    _output_filename = args.back();
    args.pop_back();

    if (!verify_output_file_safe()) {
      return false;
    }
  }

  if (!args.empty()) {
    nout << "Unexpected arguments on command line:\n";
    copy(args.begin(), args.end(), ostream_iterator<string>(nout, " "));
    nout << "\r";
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: EggWriter::post_command_line
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
bool EggWriter::
post_command_line() {
  if (!_allow_stdout && !_got_output_filename) {
    nout << "You must specify the filename to write with -o.\n";
    return false;
  }

  append_command_comment(_data);
  
  return EggBase::post_command_line();
}

////////////////////////////////////////////////////////////////////
//     Function: EggWriter::verify_output_file_safe
//       Access: Protected
//  Description: This is called when the output file is given as the
//               last parameter on the command line.  Since this is a
//               fairly dangerous way to specify the output file (it's
//               easy to accidentally overwrite an input file this
//               way), the convention is to disallow this syntax if
//               the output file already exists.
//
//               This function will test if the output file exists,
//               and issue a warning message if it does, returning
//               false.  If all is well, it will return true.
////////////////////////////////////////////////////////////////////
bool EggWriter::
verify_output_file_safe() const {
  nassertr(_got_output_filename, false);

  if (_output_filename.exists()) {
    nout << "The output filename " << _output_filename << " already exists.  "
      "If you wish to overwrite it, you must use the -o option to specify "
      "the output filename, instead of simply specifying it as the last "
      "parameter.\n";
    return false;
  }
  return true;
}
