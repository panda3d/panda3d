// Filename: dcDeclaration.cxx
// Created by:  drose (18Jun04)
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

#include "dcDeclaration.h"


////////////////////////////////////////////////////////////////////
//     Function: DCDeclaration::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
DCDeclaration::
~DCDeclaration() {
}

////////////////////////////////////////////////////////////////////
//     Function: DCDeclaration::as_class
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
DCClass *DCDeclaration::
as_class() {
  return (DCClass *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: DCDeclaration::as_class
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
const DCClass *DCDeclaration::
as_class() const {
  return (DCClass *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: DCDeclaration::as_switch
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
DCSwitch *DCDeclaration::
as_switch() {
  return (DCSwitch *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: DCDeclaration::as_switch
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
const DCSwitch *DCDeclaration::
as_switch() const {
  return (DCSwitch *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function : DCDeclaration::output
//       Access : Published, Virtual
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void DCDeclaration::
output(ostream &out) const {
  output(out, true);
}

////////////////////////////////////////////////////////////////////
//     Function : DCDeclaration::
//       Access : Published
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void DCDeclaration::
write(ostream &out, int indent_level) const {
  write(out, false, indent_level);
}
