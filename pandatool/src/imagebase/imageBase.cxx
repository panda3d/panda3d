// Filename: imageBase.cxx
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

#include "imageBase.h"

////////////////////////////////////////////////////////////////////
//     Function: ImageBase::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ImageBase::
ImageBase() {
}


////////////////////////////////////////////////////////////////////
//     Function: ImageBase::post_command_line
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool ImageBase::
post_command_line() {
  return ProgramBase::post_command_line();
}

