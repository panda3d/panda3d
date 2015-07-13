// Filename: parameterRemapBasicStringToString.cxx
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

#include "parameterRemapBasicStringToString.h"
#include "interfaceMaker.h"
#include "interrogate.h"

////////////////////////////////////////////////////////////////////
//     Function: ParameterRemapBasicStringToString::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ParameterRemapBasicStringToString::
ParameterRemapBasicStringToString(CPPType *orig_type) :
  ParameterRemapToString(orig_type)
{
  static CPPType *const_char_star_type = (CPPType *)NULL;
  if (const_char_star_type == (CPPType *)NULL) {
    const_char_star_type = parser.parse_type("const char *");
  }

  _new_type = const_char_star_type;
}

////////////////////////////////////////////////////////////////////
//     Function: ParameterRemapBasicStringToString::pass_parameter
//       Access: Public, Virtual
//  Description: Outputs an expression that converts the indicated
//               variable from the original type to the new type, for
//               passing into the actual C++ function.
////////////////////////////////////////////////////////////////////
void ParameterRemapBasicStringToString::
pass_parameter(ostream &out, const string &variable_name) {
  out << "std::string(" << variable_name << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: ParameterRemapBasicStringToString::prepare_return_expr
//       Access: Public, Virtual
//  Description: This will be called immediately before
//               get_return_expr().  It outputs whatever lines the
//               remapper needs to the function to set up its return
//               value, e.g. to declare a temporary variable or
//               something.  It should return the modified expression.
////////////////////////////////////////////////////////////////////
string ParameterRemapBasicStringToString::
prepare_return_expr(ostream &out, int indent_level, const string &expression) {
  InterfaceMaker::indent(out, indent_level)
    << "static string string_holder = " << expression << ";\n";
  return "string_holder";
}

////////////////////////////////////////////////////////////////////
//     Function: ParameterRemapBasicStringToString::get_return_expr
//       Access: Public, Virtual
//  Description: Returns an expression that evalutes to the
//               appropriate value type for returning from the
//               function, given an expression of the original type.
////////////////////////////////////////////////////////////////////
string ParameterRemapBasicStringToString::
get_return_expr(const string &expression) {
  return "string_holder.c_str()";
}

////////////////////////////////////////////////////////////////////
//     Function: ParameterRemapBasicWStringToWString::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ParameterRemapBasicWStringToWString::
ParameterRemapBasicWStringToWString(CPPType *orig_type) :
  ParameterRemapToWString(orig_type)
{
  static CPPType *const_wchar_star_type = (CPPType *)NULL;
  if (const_wchar_star_type == (CPPType *)NULL) {
    const_wchar_star_type = parser.parse_type("const wchar_t *");
  }

  _new_type = const_wchar_star_type;
}

////////////////////////////////////////////////////////////////////
//     Function: ParameterRemapBasicWStringToWString::pass_parameter
//       Access: Public, Virtual
//  Description: Outputs an expression that converts the indicated
//               variable from the original type to the new type, for
//               passing into the actual C++ function.
////////////////////////////////////////////////////////////////////
void ParameterRemapBasicWStringToWString::
pass_parameter(ostream &out, const string &variable_name) {
  out << "std::wstring(" << variable_name << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: ParameterRemapBasicWStringToWString::prepare_return_expr
//       Access: Public, Virtual
//  Description: This will be called immediately before
//               get_return_expr().  It outputs whatever lines the
//               remapper needs to the function to set up its return
//               value, e.g. to declare a temporary variable or
//               something.  It should return the modified expression.
////////////////////////////////////////////////////////////////////
string ParameterRemapBasicWStringToWString::
prepare_return_expr(ostream &out, int indent_level, const string &expression) {
  InterfaceMaker::indent(out, indent_level)
    << "static wstring string_holder = " << expression << ";\n";
  return "string_holder";
}

////////////////////////////////////////////////////////////////////
//     Function: ParameterRemapBasicWStringToWString::get_return_expr
//       Access: Public, Virtual
//  Description: Returns an expression that evalutes to the
//               appropriate value type for returning from the
//               function, given an expression of the original type.
////////////////////////////////////////////////////////////////////
string ParameterRemapBasicWStringToWString::
get_return_expr(const string &expression) {
  return "string_holder.c_str()";
}
