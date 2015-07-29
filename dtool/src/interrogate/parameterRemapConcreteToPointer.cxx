// Filename: parameterRemapConcreteToPointer.cxx
// Created by:  drose (01Aug00)
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

#include "parameterRemapConcreteToPointer.h"
#include "interrogate.h"
#include "interrogateBuilder.h"
#include "typeManager.h"

#include "cppType.h"
#include "cppDeclaration.h"
#include "cppConstType.h"
#include "cppPointerType.h"

////////////////////////////////////////////////////////////////////
//     Function: ParameterRemapConcreteToPointer::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ParameterRemapConcreteToPointer::
ParameterRemapConcreteToPointer(CPPType *orig_type) :
  ParameterRemap(orig_type)
{
  _new_type = TypeManager::wrap_pointer(orig_type);
}

////////////////////////////////////////////////////////////////////
//     Function: ParameterRemapConcreteToPointer::pass_parameter
//       Access: Public, Virtual
//  Description: Outputs an expression that converts the indicated
//               variable from the new type to the original type, for
//               passing into the actual C++ function.
////////////////////////////////////////////////////////////////////
void ParameterRemapConcreteToPointer::
pass_parameter(ostream &out, const string &variable_name) {
  if (variable_name.size() > 1 && variable_name[0] == '&') {
    // Prevent generating something like *&param
    // Also, if this is really some local type, we can presumably
    // just move it?
    out << "MOVE(" << variable_name.substr(1) << ")";
  } else {
    out << "*" << variable_name;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ParameterRemapConcreteToPointer::get_return_expr
//       Access: Public, Virtual
//  Description: Returns an expression that evalutes to the
//               appropriate value type for returning from the
//               function, given an expression of the original type.
////////////////////////////////////////////////////////////////////
string ParameterRemapConcreteToPointer::
get_return_expr(const string &expression) {
  return
    "new " + _orig_type->get_local_name(&parser) +
    "(" + expression + ")";
}

////////////////////////////////////////////////////////////////////
//     Function: ParameterRemapConcreteToPointer::return_value_needs_management
//       Access: Public, Virtual
//  Description: Returns true if the return value represents a value
//               that was newly allocated, and hence must be
//               explicitly deallocated later by the caller.
////////////////////////////////////////////////////////////////////
bool ParameterRemapConcreteToPointer::
return_value_needs_management() {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ParameterRemapConcreteToPointer::get_return_value_destructor
//       Access: Public, Virtual
//  Description: If return_value_needs_management() returns true, this
//               should return the index of the function that should
//               be called when it is time to destruct the return
//               value.  It will generally be the same as the
//               destructor for the class we just returned a pointer
//               to.
////////////////////////////////////////////////////////////////////
FunctionIndex ParameterRemapConcreteToPointer::
get_return_value_destructor() {
  return builder.get_destructor_for(_orig_type);
}

////////////////////////////////////////////////////////////////////
//     Function: ParameterRemapConcreteToPointer::return_value_should_be_simple
//       Access: Public, Virtual
//  Description: This is a hack around a problem VC++ has with
//               overly-complex expressions, particularly in
//               conjunction with the 'new' operator.  If this
//               parameter type is one that will probably give VC++ a
//               headache, this should be set true to indicate that
//               the code generator should save the return value
//               expression into a temporary variable first, and pass
//               the temporary variable name in instead.
////////////////////////////////////////////////////////////////////
bool ParameterRemapConcreteToPointer::
return_value_should_be_simple() {
  return true;
}
