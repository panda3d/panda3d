// Filename: parameterRemapBasicStringRefToString.cxx
// Created by:  drose (09Aug00)
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

#include "parameterRemapBasicStringRefToString.h"

////////////////////////////////////////////////////////////////////
//     Function: ParameterRemapBasicStringRefToString::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ParameterRemapBasicStringRefToString::
ParameterRemapBasicStringRefToString(CPPType *orig_type) :
  ParameterRemapToString(orig_type)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ParameterRemapBasicStringRefToString::pass_parameter
//       Access: Public, Virtual
//  Description: Outputs an expression that converts the indicated
//               variable from the original type to the new type, for
//               passing into the actual C++ function.
////////////////////////////////////////////////////////////////////
void ParameterRemapBasicStringRefToString::
pass_parameter(ostream &out, const string &variable_name) {
  out << variable_name;
}

////////////////////////////////////////////////////////////////////
//     Function: ParameterRemapBasicStringRefToString::get_return_expr
//       Access: Public, Virtual
//  Description: Returns an expression that evalutes to the
//               appropriate value type for returning from the
//               function, given an expression of the original type.
////////////////////////////////////////////////////////////////////
string ParameterRemapBasicStringRefToString::
get_return_expr(const string &expression) {
  return "(" + expression + ").c_str()";
}
