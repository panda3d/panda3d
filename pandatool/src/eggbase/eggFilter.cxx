// Filename: eggFilter.cxx
// Created by:  drose (14Feb00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "eggFilter.h"

////////////////////////////////////////////////////////////////////
//     Function: EggFilter::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggFilter::
EggFilter(bool allow_last_param, bool allow_stdout) :
  EggWriter(allow_last_param, allow_stdout)
{
  // The default path store for programs that read egg files and write
  // them again is PS_relative.
  _path_replace->_path_store = PS_relative;

  clear_runlines();
  if (allow_last_param) {
    add_runline("[opts] input.egg output.egg");
  }
  add_runline("[opts] -o output.egg input.egg");
  if (allow_stdout) {
    add_runline("[opts] input.egg >output.egg");
  }

  redescribe_option
    ("cs",
     "Specify the coordinate system of the resulting egg file.  This may be "
     "one of 'y-up', 'z-up', 'y-up-left', or 'z-up-left'.  The default "
     "is the same coordinate system as the input egg file.  If this is "
     "different from the input egg file, a conversion will be performed.");
}


////////////////////////////////////////////////////////////////////
//     Function: EggFilter::handle_args
//       Access: Protected, Virtual
//  Description: Does something with the additional arguments on the
//               command line (after all the -options have been
//               parsed).  Returns true if the arguments are good,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool EggFilter::
handle_args(ProgramBase::Args &args) {
  if (!check_last_arg(args, 1)) {
    return false;
  }

  if (!_got_path_directory && _got_output_filename) {
    // Put in the name of the output directory.
    _path_replace->_path_directory = _output_filename.get_dirname();
  }

  return EggReader::handle_args(args);
}

////////////////////////////////////////////////////////////////////
//     Function: EggFilter::post_command_line
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool EggFilter::
post_command_line() {
  // writer first, so we can fiddle with the _path_replace options if
  // necessary.
  return EggWriter::post_command_line() && EggReader::post_command_line();
}
