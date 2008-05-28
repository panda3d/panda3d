// Filename: eggConverter.cxx
// Created by:  drose (15Feb00)
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

#include "eggConverter.h"

////////////////////////////////////////////////////////////////////
//     Function: EggConverter::Constructor
//       Access: Public
//  Description: The first parameter to the constructor should be the
//               one-word name of the alien file format that is to be
//               read or written, for instance "OpenFlight" or
//               "Alias".  It's just used in printing error messages
//               and such.  The second parameter is the preferred
//               extension of files of this form, if any, with a
//               leading dot.
////////////////////////////////////////////////////////////////////
EggConverter::
EggConverter(const string &format_name,
             const string &preferred_extension,
             bool allow_last_param,
             bool allow_stdout) :
  EggFilter(allow_last_param, allow_stdout),
  _format_name(format_name)
{
  // Indicate the extension name we expect the user to supply for
  // output files.
  _preferred_extension = preferred_extension;
}
