// Filename: configVariableTempl.h
// Created by:  drose (20Oct04)
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

#ifndef CONFIGVARIABLETEMPL_H
#define CONFIGVARIABLETEMPL_H

#include "dtoolbase.h"
#include "configVariable.h"

////////////////////////////////////////////////////////////////////
//       Class : ConfigVariableTempl
// Description : This is a template class to define ConfigVaribleInt,
//               ConfigVariableBool, etc.
////////////////////////////////////////////////////////////////////
template<class ValueType, int EnumValue>
class ConfigVariableTempl {
PUBLISHED:
  INLINE ConfigVariableTempl(const string &name);
  INLINE ConfigVariableTempl(const string &name, ValueType default_value,
                             int trust_level = -1,
                             const string &description = string(),
                             const string &text = string());

  INLINE operator ValueType () const;
  INLINE void operator = (const ValueType &value);

