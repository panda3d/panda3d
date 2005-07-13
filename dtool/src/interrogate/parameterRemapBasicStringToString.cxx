// Filename: parameterRemapBasicStringToString.cxx
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

#include "parameterRemapBasicStringToString.h"
#include "interfaceMaker.h"

////////////////////////////////////////////////////////////////////
//     Function: ParameterRemapBasicStringToString::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ParameterRemapBasicStringToString::
ParameterRemapBasicStringToString(CPPType *orig_type) :
  ParameterRemapToString(orig_type)
{
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
  out << variable_name;
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
  ParameterRemapToString(orig_type)
{
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
  out << variable_name;
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
