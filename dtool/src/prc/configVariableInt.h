/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file configVariableInt.h
 * @author drose
 * @date 2004-10-20
 */

#ifndef CONFIGVARIABLEINT_H
#define CONFIGVARIABLEINT_H

#include "dtoolbase.h"
#include "configVariable.h"

/**
 * This is a convenience class to specialize ConfigVariable as an integer
 * type.
 */
class EXPCL_DTOOL_PRC ConfigVariableInt : public ConfigVariable {
PUBLISHED:
  INLINE ConfigVariableInt(const std::string &name);
  INLINE ConfigVariableInt(const std::string &name, int default_value,
                           const std::string &description = std::string(),
                           int flags = 0);
  INLINE ConfigVariableInt(const std::string &name, const std::string &default_value,
                           const std::string &description = std::string(),
                           int flags = 0);

  INLINE void operator = (int value);
  INLINE operator int () const;

  INLINE size_t size() const;
  INLINE int operator [] (size_t n) const;

  INLINE void set_value(int value);
  INLINE int get_value() const;
  INLINE int get_default_value() const;
  MAKE_PROPERTY(value, get_value, set_value);
  MAKE_PROPERTY(default_value, get_default_value);

  INLINE int get_word(size_t n) const;
  INLINE void set_word(size_t n, int value);

private:
  void set_default_value(int default_value);

private:
  AtomicAdjust::Integer _local_modified;
  int _cache;
};

#include "configVariableInt.I"

#endif
