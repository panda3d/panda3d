// Filename: parameterRemapReferenceToPointer.h
// Created by:  drose (01Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PARAMETERREMAPREFERENCETOPOINTER_H
#define PARAMETERREMAPREFERENCETOPOINTER_H

#include <dtoolbase.h>

#include "parameterRemap.h"

////////////////////////////////////////////////////////////////////
// 	 Class : ParameterRemapReferenceToPointer
// Description : A ParameterRemap class that handles remapping a
//               reference (or a const reference) parameter to a
//               pointer (or const pointer) parameter.
////////////////////////////////////////////////////////////////////
class ParameterRemapReferenceToPointer : public ParameterRemap {
public:
  ParameterRemapReferenceToPointer(CPPType *orig_type);

  virtual void pass_parameter(ostream &out, const string &variable_name);
  virtual string get_return_expr(const string &expression);
};

#endif
