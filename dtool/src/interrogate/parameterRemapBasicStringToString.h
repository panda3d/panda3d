// Filename: parameterRemapBasicStringToString.h
// Created by:  drose (09Aug00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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
class ParameterRemapBasicWStringToWString : public ParameterRemapToString {
public:
  ParameterRemapBasicWStringToWString(CPPType *orig_type);

  virtual void pass_parameter(ostream &out, const string &variable_name);
  virtual string prepare_return_expr(ostream &out, int indent_level,
                                     const string &expression);
  virtual string get_return_expr(const string &expression);
};

#endif
