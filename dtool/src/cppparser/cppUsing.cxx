// Filename: cppUsing.cxx
// Created by:  drose (16Nov99)
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


#include "cppUsing.h"
#include "cppIdentifier.h"

////////////////////////////////////////////////////////////////////
//     Function: CPPUsing::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPUsing::
CPPUsing(CPPIdentifier *ident, bool full_namespace, const CPPFile &file) :
  CPPDeclaration(file),
  _ident(ident), _full_namespace(full_namespace)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CPPUsing::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CPPUsing::
output(ostream &out, int, CPPScope *, bool) const {
  out << "using ";
  if (_full_namespace) {
    out << "namespace ";
  }
  out << *_ident;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPUsing::get_subtype
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPDeclaration::SubType CPPUsing::
get_subtype() const {
  return ST_using;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPUsing::as_using
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPUsing *CPPUsing::
as_using() {
  return this;
}
