// Filename: parameterRemapPTToPointer.h
// Created by:  drose (10Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PARAMETERREMAPPTTOPOINTER_H
#define PARAMETERREMAPPTTOPOINTER_H

#include <dtoolbase.h>

#include "parameterRemap.h"

class CPPType;
class CPPStructType;

////////////////////////////////////////////////////////////////////
// 	 Class : ParameterRemapPTToPointer
// Description : A ParameterRemap class that handles remapping a
//               PT(Type) or PointerTo<Type> to a Type *.
////////////////////////////////////////////////////////////////////
class ParameterRemapPTToPointer : public ParameterRemap {
public:
  ParameterRemapPTToPointer(CPPType *orig_type);

  virtual void pass_parameter(ostream &out, const string &variable_name);
  virtual string get_return_expr(const string &expression);
  virtual string temporary_to_return(const string &temporary);

private:
  CPPType *_pointer_type;
};

#endif
