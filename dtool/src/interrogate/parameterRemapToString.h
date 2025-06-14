/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file parameterRemapToString.h
 * @author drose
 * @date 2000-08-09
 */

#ifndef PARAMETERREMAPTOSTRING_H
#define PARAMETERREMAPTOSTRING_H

#include "dtoolbase.h"

#include "parameterRemap.h"

/**
 * A base class for several different remapping types that convert to an
 * atomic string class.
 *
 * The atomic string class is represented in the C interface as a (const char
 * *).  Other interfaces may be able to represent it differently, subverting
 * the code defined here.
 */
class ParameterRemapToString : public ParameterRemap {
public:
  ParameterRemapToString(CPPType *orig_type);

  virtual void pass_parameter(std::ostream &out, const std::string &variable_name);
  virtual std::string get_return_expr(const std::string &expression);

  virtual bool new_type_is_atomic_string();
};

/**
 * A base class for several different remapping types that convert to an
 * atomic string class.
 *
 * The atomic string class is represented in the C interface as a (const
 * wchar_t *).  Other interfaces may be able to represent it differently,
 * subverting the code defined here.
 */
class ParameterRemapToWString : public ParameterRemap {
public:
  ParameterRemapToWString(CPPType *orig_type);

  virtual void pass_parameter(std::ostream &out, const std::string &variable_name);
  virtual std::string get_return_expr(const std::string &expression);

  virtual bool new_type_is_atomic_string();
};

#endif
