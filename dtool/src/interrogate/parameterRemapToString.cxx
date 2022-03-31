/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file parameterRemapToString.cxx
 * @author drose
 * @date 2000-08-01
 */

#include "parameterRemapToString.h"
#include "interrogate.h"
#include "typeManager.h"

using std::string;

/**
 *
 */
ParameterRemapToString::
ParameterRemapToString(CPPType *orig_type) :
  ParameterRemap(orig_type)
{
  static CPPType *char_star_type = nullptr;
  if (char_star_type == nullptr) {
    char_star_type = parser.parse_type("char *");
  }

  static CPPType *const_char_star_type = nullptr;
  if (const_char_star_type == nullptr) {
    const_char_star_type = parser.parse_type("const char *");
  }

  if (TypeManager::is_const_char_pointer(orig_type) || TypeManager::is_string(orig_type)) {
    _new_type = const_char_star_type;
  } else {
    _new_type = char_star_type;
  }
}

/**
 * Outputs an expression that converts the indicated variable from the
 * original type to the new type, for passing into the actual C++ function.
 */
void ParameterRemapToString::
pass_parameter(std::ostream &out, const string &variable_name) {
  out << variable_name;
}

/**
 * Returns an expression that evalutes to the appropriate value type for
 * returning from the function, given an expression of the original type.
 */
string ParameterRemapToString::
get_return_expr(const string &expression) {
  return expression;
}

/**
 * Returns true if the type represented by the conversion is now the atomic
 * string type.  We have to have this crazy method for representing atomic
 * string, because there's no such type in C (and hence no corresponding
 * CPPType *).
 */
bool ParameterRemapToString::
new_type_is_atomic_string() {
  return true;
}

/**
 *
 */
ParameterRemapToWString::
ParameterRemapToWString(CPPType *orig_type) :
  ParameterRemap(orig_type)
{
  static CPPType *char_star_type = nullptr;
  if (char_star_type == nullptr) {
    char_star_type = parser.parse_type("const wchar_t *");
  }

  _new_type = char_star_type;
}

/**
 * Outputs an expression that converts the indicated variable from the
 * original type to the new type, for passing into the actual C++ function.
 */
void ParameterRemapToWString::
pass_parameter(std::ostream &out, const string &variable_name) {
  out << variable_name;
}

/**
 * Returns an expression that evalutes to the appropriate value type for
 * returning from the function, given an expression of the original type.
 */
string ParameterRemapToWString::
get_return_expr(const string &expression) {
  return expression;
}

/**
 * Returns true if the type represented by the conversion is now the atomic
 * string type.  We have to have this crazy method for representing atomic
 * string, because there's no such type in C (and hence no corresponding
 * CPPType *).
 */
bool ParameterRemapToWString::
new_type_is_atomic_string() {
  return true;
}
