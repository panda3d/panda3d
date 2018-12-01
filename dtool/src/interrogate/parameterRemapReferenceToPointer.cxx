/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file parameterRemapReferenceToPointer.cxx
 * @author drose
 * @date 2000-08-01
 */

#include "parameterRemapReferenceToPointer.h"
#include "typeManager.h"

#include "cppType.h"
#include "cppDeclaration.h"
#include "cppConstType.h"
#include "cppPointerType.h"
#include "cppReferenceType.h"

/**
 *
 */
ParameterRemapReferenceToPointer::
ParameterRemapReferenceToPointer(CPPType *orig_type) :
  ParameterRemap(orig_type)
{
  _new_type = TypeManager::wrap_pointer(TypeManager::unwrap_reference(orig_type));
}

/**
 * Outputs an expression that converts the indicated variable from the new
 * type to the original type, for passing into the actual C++ function.
 */
void ParameterRemapReferenceToPointer::
pass_parameter(std::ostream &out, const std::string &variable_name) {
  if (variable_name.size() > 1 && variable_name[0] == '&') {
    // Prevent generating something like *&param Also, if this is really some
    // local type, we can presumably just move it?  This is only relevant if
    // this parameter is an rvalue reference, but CPPParser can't know that,
    // and it might have an overload that takes an rvalue reference.  It
    // shouldn't hurt either way.
    out << "std::move(" << variable_name.substr(1) << ")";
  } else {
    out << "*" << variable_name;
  }
}

/**
 * Returns an expression that evalutes to the appropriate value type for
 * returning from the function, given an expression of the original type.
 */
std::string ParameterRemapReferenceToPointer::
get_return_expr(const std::string &expression) {
  return "&(" + expression + ")";
}
