// Filename: parameterRemapBasicStringRefToString.h
// Created by:  drose (09Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PARAMETERREMAPBASICSTRINGREFTOSTRING_H
#define PARAMETERREMAPBASICSTRINGREFTOSTRING_H

#include <dtoolbase.h>

#include "parameterRemapToString.h"

////////////////////////////////////////////////////////////////////
//       Class : ParameterRemapBasicStringRefToString
// Description : Maps a const reference to a basic_string<char> to an
//               atomic string.
////////////////////////////////////////////////////////////////////
class ParameterRemapBasicStringRefToString : public ParameterRemapToString {
public:
  ParameterRemapBasicStringRefToString(CPPType *orig_type);

  virtual void pass_parameter(ostream &out, const string &variable_name);
  virtual string get_return_expr(const string &expression);
};

#endif
