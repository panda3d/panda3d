// Filename: parameterRemapConstToNonConst.C
// Created by:  drose (04Aug00)
// 
////////////////////////////////////////////////////////////////////

#include "parameterRemapConstToNonConst.h"
#include "typeManager.h"

#include <cppConstType.h>

////////////////////////////////////////////////////////////////////
//     Function: ParameterRemapConstToNonConst::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
ParameterRemapConstToNonConst::
ParameterRemapConstToNonConst(CPPType *orig_type) :
  ParameterRemap(orig_type)
{
  _new_type = TypeManager::unwrap_const(orig_type);
}

////////////////////////////////////////////////////////////////////
//     Function: ParameterRemapConstToNonConst::pass_parameter
//       Access: Public, Virtual
//  Description: Outputs an expression that converts the indicated
//               variable from the new type to the original type, for
//               passing into the actual C++ function.
////////////////////////////////////////////////////////////////////
void ParameterRemapConstToNonConst::
pass_parameter(ostream &out, const string &variable_name) {
  out << variable_name;
}

////////////////////////////////////////////////////////////////////
//     Function: ParameterRemapConstToNonConst::get_return_expr
//       Access: Public, Virtual
//  Description: Returns an expression that evalutes to the
//               appropriate value type for returning from the
//               function, given an expression of the original type.
////////////////////////////////////////////////////////////////////
string ParameterRemapConstToNonConst::
get_return_expr(const string &expression) {
  return expression;
}
