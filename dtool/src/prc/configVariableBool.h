/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file configVariableBool.h
 * @author drose
 * @date 2004-10-20
 */

#ifndef CONFIGVARIABLEBOOL_H
#define CONFIGVARIABLEBOOL_H

#include "dtoolbase.h"
#include "configVariable.h"

/**
 * This is a convenience class to specialize ConfigVariable as a boolean type.
 */
class EXPCL_DTOOL_PRC ConfigVariableBool : public ConfigVariable {
PUBLISHED:
  INLINE ConfigVariableBool(const std::string &name);
  INLINE ConfigVariableBool(const std::string &name, bool default_value,
                            const std::string &description = std::string(), int flags = 0);
  INLINE ConfigVariableBool(const std::string &name, const std::string &default_value,
                            const std::string &description = std::string(), int flags = 0);

  INLINE void operator = (bool value);
  ALWAYS_INLINE operator bool () const;

  INLINE size_t size() const;
  INLINE bool operator [] (size_t n) const;

  INLINE void set_value(bool value);
  ALWAYS_INLINE bool get_value() const;
  INLINE bool get_default_value() const;
  MAKE_PROPERTY(value, get_value, set_value);
  MAKE_PROPERTY(default_value, get_default_value);

  INLINE bool get_word(size_t n) const;
  INLINE void set_word(size_t n, bool value);

private:
  void reload_value() const;

  mutable AtomicAdjust::Integer _local_modified;
  mutable bool _cache;
};

#include "configVariableBool.I"

#endif
