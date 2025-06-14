/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file parameterRemapHandleToInt.cxx
 * @author rdb
 * @date 2015-09-08
 */

#include "parameterRemapHandleToInt.h"
#include "interrogate.h"
#include "interrogateBuilder.h"
#include "typeManager.h"

#include "cppType.h"
#include "cppDeclaration.h"
#include "cppConstType.h"
#include "cppPointerType.h"

/**
 *
 */
ParameterRemapHandleToInt::
ParameterRemapHandleToInt(CPPType *orig_type) :
  ParameterRemap(orig_type)
{
  _new_type = TypeManager::get_int_type();
}

/**
 * Outputs an expression that converts the indicated variable from the new
 * type to the original type, for passing into the actual C++ function.
 */
void ParameterRemapHandleToInt::
pass_parameter(std::ostream &out, const std::string &variable_name) {
  CPPType *unwrapped = TypeManager::unwrap_const(_orig_type);

  if (unwrapped->get_local_name(&parser) == "TypeHandle") {
    out << "TypeHandle::from_index(" << variable_name << ")";
  } else {
    out << unwrapped->get_local_name(&parser) << "(" << variable_name << ")";
  }
}

/**
 * Returns an expression that evalutes to the appropriate value type for
 * returning from the function, given an expression of the original type.
 */
std::string ParameterRemapHandleToInt::
get_return_expr(const std::string &expression) {
  return "(" + expression + ").get_index()";
}
