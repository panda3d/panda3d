// Filename: parameterRemapEnumToInt.h
// Created by:  drose (04Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PARAMETERREMAPENUMTOINT_H
#define PARAMETERREMAPENUMTOINT_H

#include <dtoolbase.h>

#include "parameterRemap.h"

////////////////////////////////////////////////////////////////////
// 	 Class : ParameterRemapEnumToInt
// Description : A ParameterRemap class that handles remapping an
//               enumerated type to an integer parameter.
////////////////////////////////////////////////////////////////////
class ParameterRemapEnumToInt : public ParameterRemap {
public:
  ParameterRemapEnumToInt(CPPType *orig_type);

  virtual void pass_parameter(ostream &out, const string &variable_name);
  virtual string get_return_expr(const string &expression);

private:
  CPPType *_enum_type;

  CPPType *unwrap_type(CPPType *source_type) const;
};

#endif
