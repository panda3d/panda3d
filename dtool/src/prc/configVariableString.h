// Filename: configVariableString.h
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

#ifndef CONFIGVARIABLESTRING_H
#define CONFIGVARIABLESTRING_H

#include "dtoolbase.h"
#include "configVariable.h"
#include "filename.h"

////////////////////////////////////////////////////////////////////
//       Class : ConfigVariableString
// Description : This is a convenience class to specialize
//               ConfigVariable as a string type.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOLCONFIG ConfigVariableString : public ConfigVariable {
PUBLISHED:
  INLINE ConfigVariableString(const string &name);
  INLINE ConfigVariableString(const string &name, string default_value,
                              const string &description = string(), int flags = 0);

  INLINE void operator = (const string &value);
  INLINE operator string () const;
  INLINE operator Filename () const;
  INLINE bool empty() const;

  // Comparison operators are handy.
  INLINE bool operator == (const string &other) const;
  INLINE bool operator != (const string &other) const;
  INLINE bool operator < (const string &other) const;

  INLINE void set_value(const string &value);
  INLINE string get_value() const;
  INLINE string get_default_value() const;

  INLINE string get_word(int n) const;
  INLINE void set_word(int n, const string &value);
};

#include "configVariableString.I"

#endif
