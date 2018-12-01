/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file configVariableDouble.h
 * @author drose
 * @date 2004-10-20
 */

#ifndef CONFIGVARIABLEDOUBLE_H
#define CONFIGVARIABLEDOUBLE_H

#include "dtoolbase.h"
#include "configVariable.h"

/**
 * This is a convenience class to specialize ConfigVariable as a floating-
 * point type.
 */
class EXPCL_DTOOL_PRC ConfigVariableDouble : public ConfigVariable {
PUBLISHED:
  INLINE ConfigVariableDouble(const std::string &name);
  INLINE ConfigVariableDouble(const std::string &name, double default_value,
                              const std::string &description = std::string(),
                              int flags = 0);
  INLINE ConfigVariableDouble(const std::string &name, const std::string &default_value,
                              const std::string &description = std::string(),
                              int flags = 0);

  INLINE void operator = (double value);
  INLINE operator double () const;

  INLINE size_t size() const;
  INLINE double operator [] (size_t n) const;

  INLINE void set_value(double value);
  INLINE double get_value() const;
  INLINE double get_default_value() const;
  MAKE_PROPERTY(value, get_value, set_value);
  MAKE_PROPERTY(default_value, get_default_value);

  INLINE double get_word(size_t n) const;
  INLINE void set_word(size_t n, double value);

private:
  void set_default_value(double default_value);

private:
  AtomicAdjust::Integer _local_modified;
  double _cache;
};

#include "configVariableDouble.I"

#endif
