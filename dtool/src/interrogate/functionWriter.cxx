// Filename: functionWriter.cxx
// Created by:  drose (14Sep01)
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

#include "functionWriter.h"

////////////////////////////////////////////////////////////////////
//     Function: FunctionWriter::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
FunctionWriter::
FunctionWriter() {
}

////////////////////////////////////////////////////////////////////
//     Function: FunctionWriter::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
FunctionWriter::
~FunctionWriter() {
}

////////////////////////////////////////////////////////////////////
//     Function: FunctionWriter::get_name
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
const string &FunctionWriter::
get_name() const {
  return _name;
}

////////////////////////////////////////////////////////////////////
//     Function: FunctionWriter::compare_to
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
int FunctionWriter::
compare_to(const FunctionWriter &other) const {
  // Lexicographical string comparison.

  string::const_iterator n1, n2;
  n1 = _name.begin();
  n2 = other._name.begin();
  while (n1 != _name.end() && n2 != other._name.end()) {
    if (*n1 != *n2) {
      return *n1 - *n2;
    }
    ++n1;
    ++n2;
  }
  if (n1 != _name.end()) {
    return -1;
  }
  if (n2 != other._name.end()) {
    return 1;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: FunctionWriter::write_prototype
//       Access: Public, Virtual
//  Description: Outputs the prototype for the function.
////////////////////////////////////////////////////////////////////
void FunctionWriter::
write_prototype(ostream &) {
}

////////////////////////////////////////////////////////////////////
//     Function: FunctionWriter::write_code
//       Access: Public, Virtual
//  Description: Outputs the code for the function.
////////////////////////////////////////////////////////////////////
void FunctionWriter::
write_code(ostream &) {
}
