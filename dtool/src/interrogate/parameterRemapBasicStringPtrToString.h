// Filename: parameterRemapBasicStringPtrToString.h
// Created by:  drose (11Aug09)
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

#ifndef PARAMETERREMAPBASICSTRINGPTRTOSTRING_H
#define PARAMETERREMAPBASICSTRINGPTRTOSTRING_H

#include "dtoolbase.h"

#include "parameterRemapToString.h"

////////////////////////////////////////////////////////////////////
//       Class : ParameterRemapBasicStringPtrToString
// Description : Maps a const pointer to a basic_string<char> to an
//               atomic string.
////////////////////////////////////////////////////////////////////
class ParameterRemapBasicStringPtrToString : public ParameterRemapToString {
public:
  ParameterRemapBasicStringPtrToString(CPPType *orig_type);

  virtual void pass_parameter(ostream &out, const string &variable_name);
  virtual string get_return_expr(const string &expression);
};

////////////////////////////////////////////////////////////////////
//       Class : ParameterRemapBasicWStringPtrToWString
// Description : Maps a const pointer to a basic_string<wchar_t> to an
//               atomic string.
////////////////////////////////////////////////////////////////////
class ParameterRemapBasicWStringPtrToWString : public ParameterRemapToWString {
public:
  ParameterRemapBasicWStringPtrToWString(CPPType *orig_type);

  virtual void pass_parameter(ostream &out, const string &variable_name);
  virtual string get_return_expr(const string &expression);
};

#endif
