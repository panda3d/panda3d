/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file configVariableEnum.h
 * @author drose
 * @date 2004-10-21
 */

#ifndef CONFIGVARIABLEENUM_H
#define CONFIGVARIABLEENUM_H

#include "dtoolbase.h"
#include "configVariable.h"

/**
 * This class specializes ConfigVariable as an enumerated type.  It is a
 * template class, so it cannot be easily published; it's not really necessary
 * outside of C++ anyway.
 *
 * This variable assumes that the enumerated type in question has input and
 * output stream operators defined that do the right thing (outputting a
 * sensible string for the type, and converting a string to the correct
 * value).
 */
template<class EnumType>
class ConfigVariableEnum : public ConfigVariable {
public:
  INLINE ConfigVariableEnum(const std::string &name, EnumType default_value,
                            const std::string &description = std::string(),
                            int flags = 0);
  INLINE ConfigVariableEnum(const std::string &name, const std::string &default_value,
                            const std::string &description = std::string(),
                            int flags = 0);
  INLINE ~ConfigVariableEnum();

  INLINE void operator = (EnumType value);
  INLINE operator EnumType () const;

  INLINE size_t size() const;
  INLINE EnumType operator [] (size_t n) const;

  INLINE void set_value(EnumType value);
  INLINE EnumType get_value() const;
  INLINE EnumType get_default_value() const;
  MAKE_PROPERTY(value, get_value, set_value);
  MAKE_PROPERTY(default_value, get_default_value);

  INLINE EnumType get_word(size_t n) const;
  INLINE void set_word(size_t n, EnumType value);

private:
  INLINE EnumType parse_string(const std::string &value) const;
  INLINE std::string format_enum(EnumType value) const;

private:
  bool _got_default_value;
  EnumType _default_value;

  AtomicAdjust::Integer _local_modified;
  EnumType _cache;
};

#include "configVariableEnum.I"

#endif
