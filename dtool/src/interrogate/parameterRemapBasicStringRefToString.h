/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file parameterRemapBasicStringRefToString.h
 * @author drose
 * @date 2000-08-09
 */

#ifndef PARAMETERREMAPBASICSTRINGREFTOSTRING_H
#define PARAMETERREMAPBASICSTRINGREFTOSTRING_H

#include "dtoolbase.h"

#include "parameterRemapToString.h"

/**
 * Maps a const reference to a basic_string<char> to an atomic string.
 */
class ParameterRemapBasicStringRefToString : public ParameterRemapToString {
public:
  ParameterRemapBasicStringRefToString(CPPType *orig_type);

  virtual void pass_parameter(std::ostream &out, const std::string &variable_name);
  virtual std::string get_return_expr(const std::string &expression);
};

/**
 * Maps a const reference to a basic_string<wchar_t> to an atomic string.
 */
class ParameterRemapBasicWStringRefToWString : public ParameterRemapToWString {
public:
  ParameterRemapBasicWStringRefToWString(CPPType *orig_type);

  virtual void pass_parameter(std::ostream &out, const std::string &variable_name);
  virtual std::string get_return_expr(const std::string &expression);
};

#endif
