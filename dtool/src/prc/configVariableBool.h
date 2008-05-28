// Filename: configVariableBool.h
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

#ifndef CONFIGVARIABLEBOOL_H
#define CONFIGVARIABLEBOOL_H

#include "dtoolbase.h"
#include "configVariable.h"

////////////////////////////////////////////////////////////////////
//       Class : ConfigVariableBool
// Description : This is a convenience class to specialize
//               ConfigVariable as a boolean type.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOLCONFIG ConfigVariableBool : public ConfigVariable {
PUBLISHED:
  INLINE ConfigVariableBool(const string &name);
  INLINE ConfigVariableBool(const string &name, bool default_value,
                            const string &description = string(), int flags = 0);
  INLINE ConfigVariableBool(const string &name, const string &default_value,
                            const string &description = string(), int flags = 0);

  INLINE void operator = (bool value);
  INLINE operator bool () const;

  INLINE int size() const;
  INLINE bool operator [] (int n) const;

  INLINE void set_value(bool value);
  INLINE bool get_value() const;
  INLINE bool get_default_value() const;

  INLINE bool get_word(int n) const;
  INLINE void set_word(int n, bool value);

private:
  AtomicAdjust::Integer _local_modified;
  bool _cache;
};

#include "configVariableBool.I"

#endif
