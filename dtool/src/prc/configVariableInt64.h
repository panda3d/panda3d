/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file configVariableInt64.h
 * @author drose
 * @date 2007-12-19
 */

#ifndef CONFIGVARIABLEINT64_H
#define CONFIGVARIABLEINT64_H

#include "dtoolbase.h"
#include "configVariable.h"
#include "numeric_types.h"

/**
 * This is a convenience class to specialize ConfigVariable as a 64-bit
 * integer type.
 */
class EXPCL_DTOOLCONFIG ConfigVariableInt64 : public ConfigVariable {
PUBLISHED:
  INLINE ConfigVariableInt64(const string &name);
  INLINE ConfigVariableInt64(const string &name, PN_int64 default_value,
                             const string &description = string(),
                             int flags = 0);
  INLINE ConfigVariableInt64(const string &name, const string &default_value,
                             const string &description = string(),
                             int flags = 0);

  INLINE void operator = (PN_int64 value);
  INLINE operator PN_int64 () const;

  INLINE size_t size() const;
  INLINE PN_int64 operator [] (size_t n) const;

  INLINE void set_value(PN_int64 value);
  INLINE PN_int64 get_value() const;
  INLINE PN_int64 get_default_value() const;
  MAKE_PROPERTY(value, get_value, set_value);
  MAKE_PROPERTY(default_value, get_default_value);

  INLINE PN_int64 get_word(size_t n) const;
  INLINE void set_word(size_t n, PN_int64 value);

private:
  void set_default_value(PN_int64 default_value);

private:
  AtomicAdjust::Integer _local_modified;
  PN_int64 _cache;
};

#include "configVariableInt64.I"

#endif
