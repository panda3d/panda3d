/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file parameterRemapThis.h
 * @author drose
 * @date 2000-08-02
 */

#ifndef PARAMETERREMAPTHIS_H
#define PARAMETERREMAPTHIS_H

#include "dtoolbase.h"

#include "parameterRemap.h"

class CPPType;

/**
 * A ParameterRemap class that represents a generated "this" parameter.
 */
class ParameterRemapThis : public ParameterRemap {
public:
  ParameterRemapThis(CPPType *type, bool is_const);

  virtual void pass_parameter(std::ostream &out, const std::string &variable_name);
  virtual std::string get_return_expr(const std::string &expression);
  virtual bool is_this();
};

#endif
