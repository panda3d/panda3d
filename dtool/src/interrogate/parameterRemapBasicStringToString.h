// Filename: parameterRemapBasicStringToString.h
// Created by:  drose (09Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PARAMETERREMAPBASICSTRINGTOSTRING_H
#define PARAMETERREMAPBASICSTRINGTOSTRING_H

#include <dtoolbase.h>

#include "parameterRemapToString.h"

////////////////////////////////////////////////////////////////////
// 	 Class : ParameterRemapBasicStringToString
// Description : Maps a concrete basic_string<char> to an atomic
//               string.
////////////////////////////////////////////////////////////////////
class ParameterRemapBasicStringToString : public ParameterRemapToString {
public:
  ParameterRemapBasicStringToString(CPPType *orig_type);

  virtual void pass_parameter(ostream &out, const string &variable_name);
  virtual string prepare_return_expr(ostream &out, int indent_level, 
				     const string &expression);
  virtual string get_return_expr(const string &expression);
};

#endif
