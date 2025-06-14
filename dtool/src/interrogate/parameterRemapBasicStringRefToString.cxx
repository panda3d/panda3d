/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file parameterRemapBasicStringRefToString.cxx
 * @author drose
 * @date 2000-08-09
 */

#include "parameterRemapBasicStringRefToString.h"
#include "interrogate.h"

using std::string;

/**
 *
 */
ParameterRemapBasicStringRefToString::
ParameterRemapBasicStringRefToString(CPPType *orig_type) :
  ParameterRemapToString(orig_type)
{
  static CPPType *const_char_star_type = nullptr;
  if (const_char_star_type == nullptr) {
    const_char_star_type = parser.parse_type("const char *");
  }

  _new_type = const_char_star_type;
}

/**
 * Outputs an expression that converts the indicated variable from the
 * original type to the new type, for passing into the actual C++ function.
 */
void ParameterRemapBasicStringRefToString::
pass_parameter(std::ostream &out, const string &variable_name) {
  out << "std::string(" << variable_name << ")";
}

/**
 * Returns an expression that evalutes to the appropriate value type for
 * returning from the function, given an expression of the original type.
 */
string ParameterRemapBasicStringRefToString::
get_return_expr(const string &expression) {
  return "(" + expression + ").c_str()";
}

/**
 *
 */
ParameterRemapBasicWStringRefToWString::
ParameterRemapBasicWStringRefToWString(CPPType *orig_type) :
  ParameterRemapToWString(orig_type)
{
  static CPPType *const_wchar_star_type = nullptr;
  if (const_wchar_star_type == nullptr) {
    const_wchar_star_type = parser.parse_type("const wchar_t *");
  }

  _new_type = const_wchar_star_type;
}

/**
 * Outputs an expression that converts the indicated variable from the
 * original type to the new type, for passing into the actual C++ function.
 */
void ParameterRemapBasicWStringRefToWString::
pass_parameter(std::ostream &out, const string &variable_name) {
  out << "std::wstring(" << variable_name << ")";
}

/**
 * Returns an expression that evalutes to the appropriate value type for
 * returning from the function, given an expression of the original type.
 */
string ParameterRemapBasicWStringRefToWString::
get_return_expr(const string &expression) {
  return "(" + expression + ").c_str()";
}
