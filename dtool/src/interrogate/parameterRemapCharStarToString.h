// Filename: parameterRemapCharStarToString.h
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

#ifndef PARAMETERREMAPCHARSTARTOSTRING_H
#define PARAMETERREMAPCHARSTARTOSTRING_H

#include "dtoolbase.h"

#include "parameterRemapToString.h"

////////////////////////////////////////////////////////////////////
//       Class : ParameterRemapCharStarToString
// Description : Maps from (char *) or (const char *) to the atomic
//               string type.
////////////////////////////////////////////////////////////////////
class ParameterRemapCharStarToString : public ParameterRemapToString {
public:
  ParameterRemapCharStarToString(CPPType *orig_type);
};

////////////////////////////////////////////////////////////////////
//       Class : ParameterRemapWCharStarToWString
// Description : Maps from (wchar_t *) or (const wchar_ *) to the atomic
//               wide-string type.
////////////////////////////////////////////////////////////////////
class ParameterRemapWCharStarToWString : public ParameterRemapToWString {
public:
  ParameterRemapWCharStarToWString(CPPType *orig_type);
};

#endif
