// Filename: cppMakeSeq.cxx
// Created by:  drose (06Nov08)
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

#include "cppMakeSeq.h"

////////////////////////////////////////////////////////////////////
//     Function: CPPMakeSeq::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPMakeSeq::
CPPMakeSeq(CPPIdentifier *ident,
           CPPFunctionGroup *length_getter,
           CPPFunctionGroup *element_getter,
           CPPScope *current_scope, const CPPFile &file) :
  CPPDeclaration(file),
  _ident(ident),
  _length_getter(length_getter),
  _element_getter(element_getter)
{
  _ident->_native_scope = current_scope;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPMakeSeq::get_simple_name
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
string CPPMakeSeq::
get_simple_name() const {
  return _ident->get_simple_name();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPMakeSeq::get_local_name
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
string CPPMakeSeq::
get_local_name(CPPScope *scope) const {
  return _ident->get_local_name(scope);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPMakeSeq::get_fully_scoped_name
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
string CPPMakeSeq::
get_fully_scoped_name() const {
  return _ident->get_fully_scoped_name();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPMakeSeq::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CPPMakeSeq::
output(ostream &out, int indent_level, CPPScope *scope, bool complete) const {
  out << "__make_seq(" << _ident->get_local_name(scope)
      << ", " << _length_getter->_name
      << ", " << _element_getter->_name
      << ");";
}

////////////////////////////////////////////////////////////////////
//     Function: CPPMakeSeq::get_subtype
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPDeclaration::SubType CPPMakeSeq::
get_subtype() const {
  return ST_make_seq;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPMakeSeq::as_make_seq
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPMakeSeq *CPPMakeSeq::
as_make_seq() {
  return this;
}
