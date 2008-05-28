// Filename: configVariableString.h
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

#ifndef CONFIGVARIABLESTRING_H
#define CONFIGVARIABLESTRING_H

#include "dtoolbase.h"
#include "configVariable.h"

////////////////////////////////////////////////////////////////////
//       Class : ConfigVariableString
// Description : This is a convenience class to specialize
//               ConfigVariable as a string type.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOLCONFIG ConfigVariableString : public ConfigVariable {
PUBLISHED:
  INLINE ConfigVariableString(const string &name);
  INLINE ConfigVariableString(const string &name, const string &default_value,
                              const string &description = string(), int flags = 0);

  INLINE void operator = (const string &value);
  INLINE operator const string & () const;

  // These methods help the ConfigVariableString act like a C++ string
  // object.
  INLINE const char *c_str() const;
  INLINE bool empty() const;
  INLINE size_t length() const;
  INLINE char operator [] (int n) const;

  // Comparison operators are handy.
  INLINE bool operator == (const string &other) const;
  INLINE bool operator != (const string &other) const;
  INLINE bool operator < (const string &other) const;

  INLINE void set_value(const string &value);
  INLINE const string &get_value() const;
  INLINE string get_default_value() const;

  INLINE string get_word(int n) const;
  INLINE void set_word(int n, const string &value);

private:
  AtomicAdjust::Integer _local_modified;
  string _cache;
};

#include "configVariableString.I"

#endif
