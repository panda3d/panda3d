/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file parameterRemapBasicStringPtrToString.h
 * @author drose
 * @date 2009-08-11
 */

#ifndef PARAMETERREMAPBASICSTRINGPTRTOSTRING_H
#define PARAMETERREMAPBASICSTRINGPTRTOSTRING_H

#include "dtoolbase.h"

#include "parameterRemapToString.h"

/**
 * Maps a const pointer to a basic_string<char> to an atomic string.
 */
class ParameterRemapBasicStringPtrToString : public ParameterRemapToString {
public:
  ParameterRemapBasicStringPtrToString(CPPType *orig_type);

  virtual void pass_parameter(std::ostream &out, const std::string &variable_name);
  virtual std::string get_return_expr(const std::string &expression);
};

/**
 * Maps a const pointer to a basic_string<wchar_t> to an atomic string.
 */
class ParameterRemapBasicWStringPtrToWString : public ParameterRemapToWString {
public:
  ParameterRemapBasicWStringPtrToWString(CPPType *orig_type);

  virtual void pass_parameter(std::ostream &out, const std::string &variable_name);
  virtual std::string get_return_expr(const std::string &expression);
};

#endif
