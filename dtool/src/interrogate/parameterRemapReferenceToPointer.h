/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file parameterRemapReferenceToPointer.h
 * @author drose
 * @date 2000-08-01
 */

#ifndef PARAMETERREMAPREFERENCETOPOINTER_H
#define PARAMETERREMAPREFERENCETOPOINTER_H

#include "dtoolbase.h"

#include "parameterRemap.h"

/**
 * A ParameterRemap class that handles remapping a reference (or a const
 * reference) parameter to a pointer (or const pointer) parameter.
 */
class ParameterRemapReferenceToPointer : public ParameterRemap {
public:
  ParameterRemapReferenceToPointer(CPPType *orig_type);

  virtual void pass_parameter(std::ostream &out, const std::string &variable_name);
  virtual std::string get_return_expr(const std::string &expression);
};

#endif
