// Filename: cppMakeProperty.cxx
// Created by:  rdb (18Sep14)
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

#include "cppMakeProperty.h"
#include "cppFunctionGroup.h"

////////////////////////////////////////////////////////////////////
//     Function: CPPMakeProperty::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPMakeProperty::
CPPMakeProperty(CPPIdentifier *ident,
                CPPFunctionGroup *getter, CPPFunctionGroup *setter,
                CPPScope *current_scope, const CPPFile &file) :
  CPPDeclaration(file),
  _ident(ident),
  _getter(getter),
  _setter(setter)
{
  _ident->_native_scope = current_scope;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPMakeProperty::get_simple_name
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
string CPPMakeProperty::
get_simple_name() const {
  return _ident->get_simple_name();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPMakeProperty::get_local_name
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
string CPPMakeProperty::
get_local_name(CPPScope *scope) const {
  return _ident->get_local_name(scope);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPMakeProperty::get_fully_scoped_name
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
string CPPMakeProperty::
get_fully_scoped_name() const {
  return _ident->get_fully_scoped_name();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPMakeProperty::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CPPMakeProperty::
output(ostream &out, int indent_level, CPPScope *scope, bool complete) const {
  out << "__make_property(" << _ident->get_local_name(scope)
      << ", " << _getter->_name;

  if (_setter != NULL) {
    out << ", " << _setter->_name;
  }
  out << ");";
}

////////////////////////////////////////////////////////////////////
//     Function: CPPMakeProperty::get_subtype
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPDeclaration::SubType CPPMakeProperty::
get_subtype() const {
  return ST_make_property;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPMakeProperty::as_make_property
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPMakeProperty *CPPMakeProperty::
as_make_property() {
  return this;
}
