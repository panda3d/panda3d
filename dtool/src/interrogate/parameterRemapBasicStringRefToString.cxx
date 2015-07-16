// Filename: parameterRemapBasicStringRefToString.cxx
// Created by:  drose (09Aug00)
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

#include "parameterRemapBasicStringRefToString.h"
#include "interrogate.h"

////////////////////////////////////////////////////////////////////
//     Function: ParameterRemapBasicStringRefToString::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ParameterRemapBasicStringRefToString::
ParameterRemapBasicStringRefToString(CPPType *orig_type) :
  ParameterRemapToString(orig_type)
{
  static CPPType *const_char_star_type = (CPPType *)NULL;
  if (const_char_star_type == (CPPType *)NULL) {
    const_char_star_type = parser.parse_type("const char *");
  }

  _new_type = const_char_star_type;
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
  out << "std::string(" << variable_name << ")";
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

////////////////////////////////////////////////////////////////////
//     Function: ParameterRemapBasicWStringRefToWString::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ParameterRemapBasicWStringRefToWString::
ParameterRemapBasicWStringRefToWString(CPPType *orig_type) :
  ParameterRemapToWString(orig_type)
{
  static CPPType *const_wchar_star_type = (CPPType *)NULL;
  if (const_wchar_star_type == (CPPType *)NULL) {
    const_wchar_star_type = parser.parse_type("const wchar_t *");
  }

  _new_type = const_wchar_star_type;
}

////////////////////////////////////////////////////////////////////
//     Function: ParameterRemapBasicWStringRefToWString::pass_parameter
//       Access: Public, Virtual
//  Description: Outputs an expression that converts the indicated
//               variable from the original type to the new type, for
//               passing into the actual C++ function.
////////////////////////////////////////////////////////////////////
void ParameterRemapBasicWStringRefToWString::
pass_parameter(ostream &out, const string &variable_name) {
  out << "std::wstring(" << variable_name << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: ParameterRemapBasicWStringRefToWString::get_return_expr
//       Access: Public, Virtual
//  Description: Returns an expression that evalutes to the
//               appropriate value type for returning from the
//               function, given an expression of the original type.
////////////////////////////////////////////////////////////////////
string ParameterRemapBasicWStringRefToWString::
get_return_expr(const string &expression) {
  return "(" + expression + ").c_str()";
}
