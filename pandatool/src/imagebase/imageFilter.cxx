// Filename: imageFilter.cxx
// Created by:  drose (19Jun00)
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
