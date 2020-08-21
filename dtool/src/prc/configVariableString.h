/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file configVariableString.h
 * @author drose
 * @date 2004-10-20
 */

#ifndef CONFIGVARIABLESTRING_H
#define CONFIGVARIABLESTRING_H

#include "dtoolbase.h"
#include "configVariable.h"

/**
 * This is a convenience class to specialize ConfigVariable as a string type.
 */
class EXPCL_DTOOL_PRC ConfigVariableString : public ConfigVariable {
PUBLISHED:
  INLINE ConfigVariableString(const std::string &name);
  INLINE ConfigVariableString(const std::string &name, const std::string &default_value,
                              const std::string &description = std::string(), int flags = 0);

  INLINE void operator = (const std::string &value);
  INLINE operator const std::string & () const;

  // These methods help the ConfigVariableString act like a C++ string object.
  INLINE const char *c_str() const;
  INLINE bool empty() const;
  INLINE size_t length() const;
  INLINE char operator [] (size_t n) const;

  // Comparison operators are handy.
  INLINE bool operator == (const std::string &other) const;
  INLINE bool operator != (const std::string &other) const;
  INLINE bool operator < (const std::string &other) const;

  INLINE void set_value(const std::string &value);
  INLINE const std::string &get_value() const;
  INLINE std::string get_default_value() const;
  MAKE_PROPERTY(value, get_value, set_value);
  MAKE_PROPERTY(default_value, get_default_value);

  INLINE std::string get_word(size_t n) const;
  INLINE void set_word(size_t n, const std::string &value);

private:
  void reload_cache();

private:
  AtomicAdjust::Integer _local_modified;
  std::string _cache;
};

#include "configVariableString.I"

#endif
