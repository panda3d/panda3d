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

////////////////////////////////////////////////////////////////////
//     Function: ButtonHandle::get_alias
//       Access: Published
//  Description: Returns the alias (alternate name) associated with
//               the button, if any, or ButtonHandle::none() if the
//               button has no alias.
//
//               Each button is allowed to have one alias, and
//               multiple different buttons can refer to the same
//               alias.  The alias should be the more general name for
//               the button, for instance, shift is an alias for
//               lshift, but not vice-versa.
////////////////////////////////////////////////////////////////////
ButtonHandle ButtonHandle::
get_alias() const {
  if ((*this) == ButtonHandle::none()) {
    return ButtonHandle::none();
  } else {
    return ButtonRegistry::ptr()->get_alias(*this);
  }
}
