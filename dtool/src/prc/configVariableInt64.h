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
class EXPCL_DTOOL_PRC ConfigVariableInt64 : public ConfigVariable {
PUBLISHED:
  INLINE ConfigVariableInt64(const std::string &name);
  INLINE ConfigVariableInt64(const std::string &name, int64_t default_value,
                             const std::string &description = std::string(),
                             int flags = 0);
  INLINE ConfigVariableInt64(const std::string &name, const std::string &default_value,
                             const std::string &description = std::string(),
                             int flags = 0);

  INLINE void operator = (int64_t value);
  INLINE operator int64_t () const;

  INLINE size_t size() const;
  INLINE int64_t operator [] (size_t n) const;

  INLINE void set_value(int64_t value);
  INLINE int64_t get_value() const;
  INLINE int64_t get_default_value() const;
  MAKE_PROPERTY(value, get_value, set_value);
  MAKE_PROPERTY(default_value, get_default_value);

  INLINE int64_t get_word(size_t n) const;
  INLINE void set_word(size_t n, int64_t value);

private:
  void set_default_value(int64_t default_value);

private:
  AtomicAdjust::Integer _local_modified;
  int64_t _cache;
};

#include "configVariableInt64.I"

#endif
