// Filename: parameterRemapBasicStringRefToString.C
// Created by:  drose (09Aug00)
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
