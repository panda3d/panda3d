/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file withOutputFile.cxx
 * @author drose
 * @date 2001-04-11
 */

#include "withOutputFile.h"
#include "executionEnvironment.h"
#include "zStream.h"

#include "pnotify.h"

/**
 *
 */
WithOutputFile::
WithOutputFile(bool allow_last_param, bool allow_stdout,
               bool binary_output) {
  _allow_last_param = allow_last_param;
  _allow_stdout = allow_stdout;
  _binary_output = binary_output;
  _got_output_filename = false;
  _output_ptr = nullptr;
  _owns_output_ptr = false;
}

/**
 *
 */
WithOutputFile::
~WithOutputFile() {
  if (_owns_output_ptr) {
    delete _output_ptr;
    _owns_output_ptr = false;
  }
}

/**
 * Returns an output stream that corresponds to the user's intended egg file
 * output--either stdout, or the named output file.
 */
std::ostream &WithOutputFile::
get_output() {
  if (_output_ptr == nullptr) {
    if (!_got_output_filename) {
      // No filename given; use standard output.
      if (!_allow_stdout) {
        nout << "No output filename specified.\n";
        exit(1);
      }
      _output_ptr = &std::cout;
      _owns_output_ptr = false;

    } else {
      // Attempt to open the named file.
      unlink(_output_filename.c_str());
      _output_filename.make_dir();

      bool pz_file = false;
#ifdef HAVE_ZLIB
      if (_output_filename.get_extension() == "pz") {
        // The filename ends in .pz, which means to automatically compress the
        // file that we write.
        pz_file = true;
      }
#endif  // HAVE_ZLIB

      if (_binary_output || pz_file) {
        _output_filename.set_binary();
      } else {
        _output_filename.set_text();
      }

      _output_stream.clear();
      if (!_output_filename.open_write(_output_stream)) {
        nout << "Unable to write to " << _output_filename << "\n";
        exit(1);
      }
      nout << "Writing " << _output_filename << "\n";
      _output_ptr = &_output_stream;
      _owns_output_ptr = false;

#ifdef HAVE_ZLIB
      if (pz_file) {
        _output_ptr = new OCompressStream(_output_ptr, _owns_output_ptr);
        _owns_output_ptr = true;
      }
#endif  // HAVE_ZLIB
    }
  }
  return *_output_ptr;
}

/**
 * Closes the output stream previously opened by get_output().  A subsequent
 * call to get_output() will open a new stream.
 */
void WithOutputFile::
close_output() {
  if (_owns_output_ptr) {
    delete _output_ptr;
    _owns_output_ptr = false;
  }
  _output_ptr = nullptr;
  _output_stream.close();
}



/**
 * Returns true if the user specified an output filename, false otherwise
 * (e.g.  the output file is implicitly stdout).
 */
bool WithOutputFile::
has_output_filename() const {
  return _got_output_filename;
}

/**
 * If has_output_filename() returns true, this is the filename that the user
 * specified.  Otherwise, it returns the empty string.
 */
Filename WithOutputFile::
get_output_filename() const {
  if (_got_output_filename) {
    return _output_filename;
  }
  return Filename();
}

/**
 * Checks if the last filename on the argument list is a file with the
 * expected extension (if _allow_last_param was set true), and removes it from
 * the argument list if it is.  Returns true if the arguments are good, false
 * if something is invalid.
 *
 * minimum_args is the number of arguments we know must be input parameters
 * and therefore cannot be interpreted as output filenames.
 */
bool WithOutputFile::
check_last_arg(ProgramBase::Args &args, int minimum_args) {
  if (_allow_last_param && !_got_output_filename &&
      (int)args.size() > minimum_args) {
    Filename filename = Filename::from_os_specific(args.back());

    if (!_preferred_extension.empty() &&
        ("." + filename.get_extension()) != _preferred_extension) {
      // This argument must not be an output filename.
      if (!_allow_stdout) {
        nout << "Output filename " << filename
             << " does not end in " << _preferred_extension
             << ".  If this is really what you intended, "
          "use the -o output_file syntax.\n";
        return false;
      }

    } else {
      // This argument appears to be an output filename.
      _got_output_filename = true;
      _output_filename = filename;
      args.pop_back();

      if (!verify_output_file_safe()) {
        return false;
      }
    }
  }

  return true;
}

/**
 * This is called when the output file is given as the last parameter on the
 * command line.  Since this is a fairly dangerous way to specify the output
 * file (it's easy to accidentally overwrite an input file this way), the
 * convention is to disallow this syntax if the output file already exists.
 *
 * This function will test if the output file exists, and issue a warning
 * message if it does, returning false.  If all is well, it will return true.
 */
bool WithOutputFile::
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
