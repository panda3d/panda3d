// Filename: parameterRemapToString.h
// Created by:  drose (09Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PARAMETERREMAPTOSTRING_H
#define PARAMETERREMAPTOSTRING_H

#include <dtoolbase.h>

#include "parameterRemap.h"

////////////////////////////////////////////////////////////////////
//       Class : ParameterRemapToString
// Description : A base class for several different remapping types
//               that convert to an atomic string class.
//
//               The atomic string class is represented in the C
//               interface as a (const char *).  Other interfaces may
//               be able to represent it differently, subverting the
//               code defined here.
////////////////////////////////////////////////////////////////////
class ParameterRemapToString : public ParameterRemap {
public:
  ParameterRemapToString(CPPType *orig_type);

  virtual void pass_parameter(ostream &out, const string &variable_name);
  virtual string get_return_expr(const string &expression);

  virtual bool new_type_is_atomic_string();
};

#endif
