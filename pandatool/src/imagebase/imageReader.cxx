// Filename: imageReader.cxx
// Created by:  drose (19Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "imageReader.h"

////////////////////////////////////////////////////////////////////
//     Function: ImageReader::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ImageReader::
ImageReader() {
  clear_runlines();
  add_runline("[opts] imagename");
}

////////////////////////////////////////////////////////////////////
//     Function: ImageReader::handle_args
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool ImageReader::
handle_args(ProgramBase::Args &args) {
  if (args.empty()) {
    nout << "You must specify the image file to read on the command line.\n";
    return false;
  }

  if (args.size() > 1) {
    nout << "Specify only one image on the command line.\n";
    return false;
  }

  if (!_image.read(args[0])) {
    nout << "Unable to read image file " << args[0] << ".\n";
    exit(1);
  }

  return true;
}
