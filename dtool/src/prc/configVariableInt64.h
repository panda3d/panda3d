// Filename: configVariableInt64.h
// Created by:  drose (19Dec07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// aint64 with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maint64ainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef CONFIGVARIABLEINT64_H
#define CONFIGVARIABLEINT64_H

#include "dtoolbase.h"
#include "configVariable.h"
#include "numeric_types.h"

////////////////////////////////////////////////////////////////////
//       Class : ConfigVariableInt64
// Description : This is a convenience class to specialize
//               ConfigVariable as a 64-bit integer type.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOLCONFIG ConfigVariableInt64 : public ConfigVariable {
PUBLISHED:
  INLINE ConfigVariableInt64(const string &name);
  INLINE ConfigVariableInt64(const string &name, PN_int64 default_value,
                             const string &description = string(), 
                             PN_int64 flags = 0);
  INLINE ConfigVariableInt64(const string &name, const string &default_value,
                             const string &description = string(), 
                             PN_int64 flags = 0);

  INLINE void operator = (PN_int64 value);
  INLINE operator PN_int64 () const;

  INLINE PN_int64 size() const;
  INLINE PN_int64 operator [] (int n) const;

  INLINE void set_value(PN_int64 value);
  INLINE PN_int64 get_value() const;
  INLINE PN_int64 get_default_value() const;

  INLINE PN_int64 get_word(int n) const;
  INLINE void set_word(int n, PN_int64 value);

private:
  void set_default_value(PN_int64 default_value);

private:
  AtomicAdjust::Integer _local_modified;
  PN_int64 _cache;
};

#include "configVariableInt64.I"

#endif
