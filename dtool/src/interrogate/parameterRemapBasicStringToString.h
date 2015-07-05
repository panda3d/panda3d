// Filename: parameterRemapBasicStringToString.h
// Created by:  drose (09Aug00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef PARAMETERREMAPBASICSTRINGTOSTRING_H
#define PARAMETERREMAPBASICSTRINGTOSTRING_H

#include "dtoolbase.h"

#include "parameterRemapToString.h"

////////////////////////////////////////////////////////////////////
//       Class : ParameterRemapBasicStringToString
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

////////////////////////////////////////////////////////////////////
//       Class : ParameterRemapBasicWStringToWString
// Description : Maps a concrete basic_string<wchar_t> to an atomic
//               string.
////////////////////////////////////////////////////////////////////
class ParameterRemapBasicWStringToWString : public ParameterRemapToWString {
public:
  ParameterRemapBasicWStringToWString(CPPType *orig_type);

  virtual void pass_parameter(ostream &out, const string &variable_name);
  virtual string prepare_return_expr(ostream &out, int indent_level,
                                     const string &expression);
  virtual string get_return_expr(const string &expression);
};

#endif
