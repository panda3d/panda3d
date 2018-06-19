/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file configVariable.h
 * @author drose
 * @date 2004-10-18
 */

#ifndef CONFIGVARIABLE_H
#define CONFIGVARIABLE_H

#include "dtoolbase.h"
#include "configVariableBase.h"
#include "numeric_types.h"

/**
 * This is a generic, untyped ConfigVariable.  It is also the base class for
 * the typed ConfigVariables, and contains all of the code common to
 * ConfigVariables of all types (except ConfigVariableList, which is a bit of
 * a special case).
 *
 * Mostly, this class serves as a thin wrapper around ConfigVariableCore
 * and/or ConfigDeclaration, more or less duplicating the interface presented
 * there.
 */
class EXPCL_DTOOL_PRC ConfigVariable : public ConfigVariableBase {
protected:
  INLINE ConfigVariable(const std::string &name, ValueType type);
  INLINE ConfigVariable(const std::string &name, ValueType type,
                        const std::string &description, int flags);

PUBLISHED:
  INLINE explicit ConfigVariable(const std::string &name);
  INLINE ~ConfigVariable();

  INLINE const std::string &get_string_value() const;
  INLINE void set_string_value(const std::string &value);
  INLINE void clear_value();

  INLINE size_t get_num_words() const;

protected:
  INLINE const ConfigDeclaration *get_default_value() const;

  INLINE bool has_string_word(size_t n) const;
  INLINE bool has_bool_word(size_t n) const;
  INLINE bool has_int_word(size_t n) const;
  INLINE bool has_int64_word(size_t n) const;
  INLINE bool has_double_word(size_t n) const;

  INLINE std::string get_string_word(size_t n) const;
  INLINE bool get_bool_word(size_t n) const;
  INLINE int get_int_word(size_t n) const;
  INLINE int64_t get_int64_word(size_t n) const;
  INLINE double get_double_word(size_t n) const;

  INLINE void set_string_word(size_t n, const std::string &value);
  INLINE void set_bool_word(size_t n, bool value);
  INLINE void set_int_word(size_t n, int value);
  INLINE void set_int64_word(size_t n, int64_t value);
  INLINE void set_double_word(size_t n, double value);

protected:
  INLINE bool is_constructed() const;
  void report_unconstructed() const;
};

#include "configVariable.I"

#endif
