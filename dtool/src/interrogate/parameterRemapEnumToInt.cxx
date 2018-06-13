/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file parameterRemapEnumToInt.cxx
 * @author drose
 * @date 2000-08-04
 */

#include "parameterRemapEnumToInt.h"
#include "interrogate.h"

#include "cppSimpleType.h"
#include "cppConstType.h"
#include "cppPointerType.h"
#include "cppReferenceType.h"

/**
 *
 */
ParameterRemapEnumToInt::
ParameterRemapEnumToInt(CPPType *orig_type) :
  ParameterRemap(orig_type)
{
  _new_type = CPPType::new_type(new CPPSimpleType(CPPSimpleType::T_int));
  _enum_type = unwrap_type(_orig_type);
}

/**
 * Outputs an expression that converts the indicated variable from the new
 * type to the original type, for passing into the actual C++ function.
 */
void ParameterRemapEnumToInt::
pass_parameter(std::ostream &out, const std::string &variable_name) {
  out << "(" << _enum_type->get_local_name(&parser) << ")" << variable_name;
}

/**
 * Returns an expression that evalutes to the appropriate value type for
 * returning from the function, given an expression of the original type.
 */
std::string ParameterRemapEnumToInt::
get_return_expr(const std::string &expression) {
  return "(int)(" + expression + ")";
}

/**
 * Recursively walks through the type definition, and finds the enum
 * definition under all the wrappers.
 */
CPPType *ParameterRemapEnumToInt::
unwrap_type(CPPType *source_type) const {
  switch (source_type->get_subtype()) {
  case CPPDeclaration::ST_const:
    return unwrap_type(source_type->as_const_type()->_wrapped_around);

  case CPPDeclaration::ST_reference:
    return unwrap_type(source_type->as_reference_type()->_pointing_at);

  case CPPDeclaration::ST_pointer:
    return unwrap_type(source_type->as_pointer_type()->_pointing_at);

  default:
    return source_type;
  }
}
