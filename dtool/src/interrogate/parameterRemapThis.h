// Filename: parameterRemapThis.h
// Created by:  drose (02Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PARAMETERREMAPTHIS_H
#define PARAMETERREMAPTHIS_H

#include <dtoolbase.h>

#include "parameterRemap.h"

class CPPStructType;

////////////////////////////////////////////////////////////////////
//       Class : ParameterRemapThis
// Description : A ParameterRemap class that represents a generated
//               "this" parameter.
////////////////////////////////////////////////////////////////////
class ParameterRemapThis : public ParameterRemap {
public:
  ParameterRemapThis(CPPStructType *type, bool is_const);

  virtual void pass_parameter(ostream &out, const string &variable_name);
  virtual string get_return_expr(const string &expression);
  virtual bool is_this();
};

#endif
