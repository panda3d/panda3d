// Filename: parameterRemapToString.cxx
// Created by:  drose (01Aug00)
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

#include "parameterRemapToString.h"
#include "interrogate.h"

////////////////////////////////////////////////////////////////////
//     Function: ParameterRemapToString::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ParameterRemapToString::
ParameterRemapToString(CPPType *orig_type) :
  ParameterRemap(orig_type)
{
  static CPPType *char_star_type = (CPPType *)NULL;
  if (char_star_type == (CPPType *)NULL) {
    char_star_type = parser.parse_type("const char *");
  }

  _new_type = char_star_type;
}

////////////////////////////////////////////////////////////////////
//     Function: ParameterRemapToString::pass_parameter
//       Access: Public, Virtual
//  Description: Outputs an expression that converts the indicated
//               variable from the original type to the new type, for
//               passing into the actual C++ function.
////////////////////////////////////////////////////////////////////
void ParameterRemapToString::
pass_parameter(ostream &out, const string &variable_name) {
  out << variable_name;
}

////////////////////////////////////////////////////////////////////
//     Function: ParameterRemapToString::get_return_expr
//       Access: Public, Virtual
//  Description: Returns an expression that evalutes to the
//               appropriate value type for returning from the
//               function, given an expression of the original type.
////////////////////////////////////////////////////////////////////
string ParameterRemapToString::
get_return_expr(const string &expression) {
  return expression;
}

////////////////////////////////////////////////////////////////////
//     Function: ParameterRemapToString::new_type_is_atomic_string
//       Access: Public, Virtual
//  Description: Returns true if the type represented by the
//               conversion is now the atomic string type.  We have to
//               have this crazy method for representing atomic
//               string, because there's no such type in C (and hence
//               no corresponding CPPType *).
////////////////////////////////////////////////////////////////////
bool ParameterRemapToString::
new_type_is_atomic_string() {
  return true;
}
