// Filename: imageFilter.cxx
// Created by:  drose (19Jun00)
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

#include "imageFilter.h"

////////////////////////////////////////////////////////////////////
//     Function: ImageFilter::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ImageFilter::
ImageFilter(bool allow_last_param) :
  ImageWriter(allow_last_param)
{
  clear_runlines();
  if (_allow_last_param) {
    add_runline("[opts] inputimage outputimage");
  }
  add_runline("[opts] -o outputimage inputimage");
}

////////////////////////////////////////////////////////////////////
//     Function: ImageFilter::handle_args
//       Access: Protected, Virtual
//  Description: Does something with the additional arguments on the
//               command line (after all the -options have been
//               parsed).  Returns true if the arguments are good,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool ImageFilter::
handle_args(ProgramBase::Args &args) {
  if (!check_last_arg(args, 1)) {
    return false;
  }

  return ImageReader::handle_args(args);
}
