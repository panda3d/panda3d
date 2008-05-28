// Filename: cppUsing.cxx
// Created by:  drose (16Nov99)
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
