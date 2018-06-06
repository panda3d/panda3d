/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file parameterRemapPTToPointer.h
 * @author drose
 * @date 2000-08-10
 */

#ifndef PARAMETERREMAPPTTOPOINTER_H
#define PARAMETERREMAPPTTOPOINTER_H

#include "dtoolbase.h"

#include "parameterRemap.h"

class CPPType;
class CPPStructType;

/**
 * A ParameterRemap class that handles remapping a PT(Type) or PointerTo<Type>
 * to a Type *.
 */
class ParameterRemapPTToPointer : public ParameterRemap {
public:
  ParameterRemapPTToPointer(CPPType *orig_type);

  virtual void pass_parameter(std::ostream &out, const std::string &variable_name);
  virtual std::string get_return_expr(const std::string &expression);
  virtual std::string temporary_to_return(const std::string &temporary);

private:
  CPPType *_pointer_type;
};

#endif
