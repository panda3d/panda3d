// Filename: parameterRemapConcreteToPointer.h
// Created by:  drose (01Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PARAMETERREMAPCONCRETETOPOINTER_H
#define PARAMETERREMAPCONCRETETOPOINTER_H

#include <dtoolbase.h>

#include "parameterRemap.h"

////////////////////////////////////////////////////////////////////
//       Class : ParameterRemapConcreteToPointer
// Description : A ParameterRemap class that handles remapping a
//               concrete structure or class parameter to a pointer
//               parameter.
////////////////////////////////////////////////////////////////////
class ParameterRemapConcreteToPointer : public ParameterRemap {
public:
  ParameterRemapConcreteToPointer(CPPType *orig_type);

  virtual void pass_parameter(ostream &out, const string &variable_name);
  virtual string get_return_expr(const string &expression);
  virtual bool return_value_needs_management();
  virtual FunctionIndex get_return_value_destructor();
  virtual bool return_value_should_be_simple();
};

#endif
