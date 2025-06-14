/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file parameterRemapBasicStringToString.h
 * @author drose
 * @date 2000-08-09
 */

#ifndef PARAMETERREMAPBASICSTRINGTOSTRING_H
#define PARAMETERREMAPBASICSTRINGTOSTRING_H

#include "dtoolbase.h"

#include "parameterRemapToString.h"

/**
 * Maps a concrete basic_string<char> to an atomic string.
 */
class ParameterRemapBasicStringToString : public ParameterRemapToString {
public:
  ParameterRemapBasicStringToString(CPPType *orig_type);

  virtual void pass_parameter(std::ostream &out, const std::string &variable_name);
  virtual std::string prepare_return_expr(std::ostream &out, int indent_level,
                                     const std::string &expression);
  virtual std::string get_return_expr(const std::string &expression);
};

/**
 * Maps a concrete basic_string<wchar_t> to an atomic string.
 */
class ParameterRemapBasicWStringToWString : public ParameterRemapToWString {
public:
  ParameterRemapBasicWStringToWString(CPPType *orig_type);

  virtual void pass_parameter(std::ostream &out, const std::string &variable_name);
  virtual std::string prepare_return_expr(std::ostream &out, int indent_level,
                                     const std::string &expression);
  virtual std::string get_return_expr(const std::string &expression);
};

#endif
