// Filename: dcTypedef.cxx
// Created by:  drose (17Jun04)
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

#include "dcParameter.h"
#include "hashGenerator.h"
#include "dcParameter.h"
#include "dcindent.h"

////////////////////////////////////////////////////////////////////
//     Function: DCTypedef::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
DCTypedef::
DCTypedef(DCParameter *parameter) :
  _parameter(parameter),
  _number(-1)
{
}

////////////////////////////////////////////////////////////////////
//     Function: DCTypedef::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
DCTypedef::
~DCTypedef() {
  delete _parameter;
}

////////////////////////////////////////////////////////////////////
//     Function: DCTypedef::get_number
//       Access: Published
//  Description: Returns a unique index number associated with this
//               typedef definition.  This is defined implicitly when
//               the .dc file(s) are read.
////////////////////////////////////////////////////////////////////
int DCTypedef::
get_number() const {
  return _number;
}

////////////////////////////////////////////////////////////////////
//     Function: DCTypedef::get_name
//       Access: Published
//  Description: Returns the name of this typedef.
////////////////////////////////////////////////////////////////////
const string &DCTypedef::
get_name() const {
  return _parameter->get_name();
}

////////////////////////////////////////////////////////////////////
//     Function: DCTypedef::get_description
//       Access: Published
//  Description: Returns a brief decription of the typedef, useful for
//               human consumption.
////////////////////////////////////////////////////////////////////
string DCTypedef::
get_description() const {
  ostringstream strm;
  _parameter->output(strm, true);
  return strm.str();
}

////////////////////////////////////////////////////////////////////
//     Function: DCTypedef::make_new_parameter
//       Access: Public
//  Description: Returns a newly-allocated DCParameter object that
//               uses the same type as that named by the typedef.
////////////////////////////////////////////////////////////////////
DCParameter *DCTypedef::
make_new_parameter() const {
  DCParameter *new_parameter = _parameter->make_copy();
  new_parameter->set_name(string());
  new_parameter->set_typedef(this);
  return new_parameter;
}

////////////////////////////////////////////////////////////////////
//     Function: DCTypedef::set_number
//       Access: Public
//  Description: Assigns the unique number to this typedef.  This is
//               normally called only by the DCFile interface as the
//               typedef is added.
////////////////////////////////////////////////////////////////////
void DCTypedef::
set_number(int number) {
  _number = number;
}

////////////////////////////////////////////////////////////////////
//     Function: DCTypedef::write
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void DCTypedef::
write(ostream &out, bool brief, int indent_level) const {
  indent(out, indent_level)
    << "typedef ";

  // We need to preserve the parameter name in the typedef (this is
  // the typedef name); hence, we pass brief = false to output().
  _parameter->output(out, false);
  out << ";";

  if (!brief) {
    out << "  // typedef " << _number;
  }
  out << "\n";
}

