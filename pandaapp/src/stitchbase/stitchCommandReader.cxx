// Filename: stitchCommandReader.cxx
// Created by:  drose (16Mar00)
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

#include "stitchCommandReader.h"

////////////////////////////////////////////////////////////////////
//     Function: StitchCommandReader::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
StitchCommandReader::
StitchCommandReader() {
  clear_runlines();
  add_runline("[opts] input.st");
}


////////////////////////////////////////////////////////////////////
//     Function: StitchCommandReader::handle_args
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool StitchCommandReader::
handle_args(ProgramBase::Args &args) {
  if (args.empty()) {
    nout << "You must specify the stitch command file to read on the\n"
         << "command line.\n";
    return false;
  }
  if (args.size() > 1) {
    nout << "You must specify only one stitch command file to read on the\n"
         << "command line.\n";
    return false;
  }

  return _command_file.read(args[0]);
}
