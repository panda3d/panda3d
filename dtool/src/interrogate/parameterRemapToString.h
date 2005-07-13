// Filename: parameterRemapToString.h
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

#ifndef PARAMETERREMAPTOSTRING_H
#define PARAMETERREMAPTOSTRING_H

#include "dtoolbase.h"

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

////////////////////////////////////////////////////////////////////
//       Class : ParameterRemapToWString
// Description : A base class for several different remapping types
//               that convert to an atomic string class.
//
//               The atomic string class is represented in the C
//               interface as a (const wchar_t *).  Other interfaces
//               may be able to represent it differently, subverting
//               the code defined here.
////////////////////////////////////////////////////////////////////
class ParameterRemapToWString : public ParameterRemap {
public:
  ParameterRemapToWString(CPPType *orig_type);

  virtual void pass_parameter(ostream &out, const string &variable_name);
  virtual string get_return_expr(const string &expression);

  virtual bool new_type_is_atomic_string();
};

#endif
