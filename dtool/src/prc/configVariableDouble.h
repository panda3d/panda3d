// Filename: configVariableDouble.h
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

#ifndef CONFIGVARIABLEDOUBLE_H
#define CONFIGVARIABLEDOUBLE_H

#include "dtoolbase.h"
#include "configVariable.h"

////////////////////////////////////////////////////////////////////
//       Class : ConfigVariableDouble
// Description : This is a convenience class to specialize
//               ConfigVariable as a floating-point type.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOLCONFIG ConfigVariableDouble : public ConfigVariable {
PUBLISHED:
  INLINE ConfigVariableDouble(const string &name);
  INLINE ConfigVariableDouble(const string &name, double default_value,
                              const string &description = string(), 
                              int flags = 0);
  INLINE ConfigVariableDouble(const string &name, const string &default_value,
                              const string &description = string(), 
                              int flags = 0);

  INLINE void operator = (double value);
  INLINE operator double () const;

  INLINE int size() const;
  INLINE double operator [] (int n) const;

  INLINE void set_value(double value);
  INLINE double get_value() const;
  INLINE double get_default_value() const;

  INLINE double get_word(int n) const;
  INLINE void set_word(int n, double value);

private:
  void set_default_value(double default_value);
};

#include "configVariableDouble.I"

#endif
