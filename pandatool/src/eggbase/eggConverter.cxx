// Filename: eggConverter.cxx
// Created by:  drose (15Feb00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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
