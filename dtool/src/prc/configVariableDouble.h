// Filename: configVariableDouble.h
// Created by:  drose (20Oct04)
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

private:
  AtomicAdjust::Integer _local_modified;
  double _cache;
};

#include "configVariableDouble.I"

#endif
