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
CPPMakeSeq(const string &seq_name, const string &num_name,
           const string &element_name, const CPPFile &file) :
  CPPDeclaration(file),
  _seq_name(seq_name),
  _num_name(num_name),
  _element_name(element_name)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CPPMakeSeq::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CPPMakeSeq::
output(ostream &out, int indent_level, CPPScope *scope, bool complete) const {
  out << "__make_seq(" << _seq_name << ", " << _num_name << ", "
      << _element_name << ");";
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
