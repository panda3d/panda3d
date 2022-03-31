/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggMultiFilter.cxx
 * @author drose
 * @date 2000-11-02
 */

#include "eggMultiFilter.h"

#include "pnotify.h"
#include "eggData.h"

/**
 *
 */
EggMultiFilter::
EggMultiFilter(bool allow_empty) : _allow_empty(allow_empty) {
  clear_runlines();
  add_runline("-o output.egg [opts] input.egg");
  add_runline("-d dirname [opts] file.egg [file.egg ...]");
  add_runline("-inplace [opts] file.egg [file.egg ...]");
  add_runline("-inf input_list_filename [opts]");

  add_option
    ("o", "filename", 50,
     "Specify the filename to which the resulting egg file will be written.  "
     "This is only valid when there is only one input egg file on the command "
     "line.  If you want to process multiple files simultaneously, you must "
     "use either -d or -inplace.",
     &EggMultiFilter::dispatch_filename, &_got_output_filename, &_output_filename);

  add_option
    ("d", "dirname", 50,
     "Specify the name of the directory in which to write the resulting egg "
     "files.  If you are processing only one egg file, this may be omitted "
     "in lieu of the -o option.  If you are processing multiple egg files, "
     "this may be omitted only if you specify -inplace instead.",
     &EggMultiFilter::dispatch_filename, &_got_output_dirname, &_output_dirname);

  add_option
    ("inplace", "", 50,
     "If this option is given, the input egg files will be rewritten in "
     "place with the results.  This obviates the need to specify -d "
     "for an output directory; however, it's risky because the original "
     "input egg files are lost.",
     &EggMultiFilter::dispatch_none, &_inplace);

  add_option
    ("inf", "filename", 95,
     "Reads input args from a text file instead of the command line.  "
     "Useful for really, really large lists of args that break the "
     "OS-imposed limits on the length of command lines.",
     &EggMultiFilter::dispatch_filename, &_got_input_filename, &_input_filename);

  // Derived programs will set this true when they discover some command-line
  // option that will prevent the program from generating output.  This
  // removes some checks for an output specification in handle_args.
  _read_only = false;
}


/**
 * Does something with the additional arguments on the command line (after all
 * the -options have been parsed).  Returns true if the arguments are good,
 * false otherwise.
 */
bool EggMultiFilter::
handle_args(ProgramBase::Args &args) {
  if (_got_input_filename) {
    nout << "Populating args from input file: " << _input_filename << "\n";
    // Makes sure the file is set is_text
    _filename = Filename::text_filename(_input_filename);
    std::ifstream input;
    if (!_filename.open_read(input)) {
      nout << "Error opening file: " << _input_filename << "\n";
      return false;
    }
    std::string line;
    // File should be a space-delimited list of egg files
    while (std::getline(input, line, ' ')) {
      args.push_back(line);
    }
  }
  if (args.empty()) {
    if (!_allow_empty) {
      nout << "You must specify the egg file(s) to read on the command line.\n";
      return false;
    }
  } else {
    // These only apply if we have specified any egg files.
    if (_got_output_filename && args.size() == 1) {
      if (_got_output_dirname) {
        nout << "Cannot specify both -o and -d.\n";
        return false;
      } else if (_inplace) {
        nout << "Cannot specify both -o and -inplace.\n";
        return false;
      }

    } else {
      if (_got_output_filename) {
        nout << "Cannot use -o when multiple egg files are specified.\n";
        return false;
      }

      if (_got_output_dirname && _inplace) {
        nout << "Cannot specify both -inplace and -d.\n";
        return false;

      } else if (!_got_output_dirname && !_inplace) {
        if (!_read_only) {
          nout << "You must specify either -inplace or -d.\n";
          return false;
        }
      }
    }
  }

  // We need to set up _path_replace before we call read_egg().
  if (!_got_path_directory) {
    // Put in the name of the output directory.
    if (_got_output_filename) {
      _path_replace->_path_directory = _output_filename.get_dirname();
    } else if (_got_output_dirname) {
      _path_replace->_path_directory = _output_dirname;
    }
  }

  Args::const_iterator ai;
  for (ai = args.begin(); ai != args.end(); ++ai) {
    PT(EggData) data = read_egg(Filename::from_os_specific(*ai));
    if (data == nullptr) {
      // Rather than returning false, we simply exit here, so the ProgramBase
      // won't try to tell the user how to run the program just because we got
      // a bad egg file.
      exit(1);
    }

    _eggs.push_back(data);
  }

  return true;
}

/**
 *
 */
bool EggMultiFilter::
post_command_line() {
  Eggs::iterator ei;
  for (ei = _eggs.begin(); ei != _eggs.end(); ++ei) {
    EggData *data = (*ei);
    if (_got_coordinate_system) {
      data->set_coordinate_system(_coordinate_system);
    }
    append_command_comment(data);
  }

  return EggMultiBase::post_command_line();
}

/**
 * Returns the output filename of the egg file with the given input filename.
 * This is based on the user's choice of -inplace, -o, or -d.
 */
Filename EggMultiFilter::
get_output_filename(const Filename &source_filename) const {
  if (_got_output_filename) {
    nassertr(!_inplace && !_got_output_dirname && _eggs.size() == 1, Filename());
    return _output_filename;

  } else if (_got_output_dirname) {
    nassertr(!_inplace, Filename());
    Filename result = source_filename;
    result.set_dirname(_output_dirname);
    return result;
  }

  nassertr(_inplace, Filename());
  return source_filename;
}

/**
 * Writes out all of the egg files in the _eggs vector, to the output
 * directory if one is specified, or over the input files if -inplace was
 * specified.
 */
void EggMultiFilter::
write_eggs() {
  nassertv(!_read_only);
  post_process_egg_files();
  Eggs::iterator ei;
  for (ei = _eggs.begin(); ei != _eggs.end(); ++ei) {
    EggData *data = (*ei);
    Filename filename = get_output_filename(data->get_egg_filename());

    nout << "Writing " << filename << "\n";
    filename.make_dir();
    if (!data->write_egg(filename)) {
      // Error writing an egg file; abort.
      exit(1);
    }
  }
}
