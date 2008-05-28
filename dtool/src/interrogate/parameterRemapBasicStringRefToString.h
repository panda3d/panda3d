// Filename: parameterRemapBasicStringRefToString.h
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

#ifndef PARAMETERREMAPBASICSTRINGREFTOSTRING_H
#define PARAMETERREMAPBASICSTRINGREFTOSTRING_H

#include "dtoolbase.h"

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

////////////////////////////////////////////////////////////////////
//       Class : ParameterRemapBasicWStringRefToWString
// Description : Maps a const reference to a basic_string<wchar_t> to an
//               atomic string.
////////////////////////////////////////////////////////////////////
class ParameterRemapBasicWStringRefToWString : public ParameterRemapToWString {
public:
  ParameterRemapBasicWStringRefToWString(CPPType *orig_type);

  virtual void pass_parameter(ostream &out, const string &variable_name);
  virtual string get_return_expr(const string &expression);
};

#endif
