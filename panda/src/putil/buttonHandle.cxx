// Filename: buttonHandle.cxx
// Created by:  drose (01Mar00)
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

#include "buttonHandle.h"
#include "buttonRegistry.h"

// This is initialized to zero by static initialization.
ButtonHandle ButtonHandle::_none;


////////////////////////////////////////////////////////////////////
//     Function: ButtonHandle::get_name
//       Access: Public
//  Description: Returns the name of the button.
////////////////////////////////////////////////////////////////////
string ButtonHandle::
get_name() const {
  if ((*this) == ButtonHandle::none()) {
    return "none";
  } else {
    return ButtonRegistry::ptr()->get_name(*this);
  }
}
