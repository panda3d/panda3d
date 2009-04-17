// Filename: configVariable.h
// Created by:  drose (18Oct04)
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

#ifndef CONFIGVARIABLE_H
#define CONFIGVARIABLE_H

#include "dtoolbase.h"
#include "configVariableBase.h"
#include "numeric_types.h"

////////////////////////////////////////////////////////////////////
//       Class : ConfigVariable
// Description : This is a generic, untyped ConfigVariable.  It is
//               also the base class for the typed ConfigVariables,
//               and contains all of the code common to
//               ConfigVariables of all types (except
//               ConfigVariableList, which is a bit of a special
//               case).
//
//               Mostly, this class serves as a thin wrapper around
//               ConfigVariableCore and/or ConfigDeclaration, more or
//               less duplicating the interface presented there.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOLCONFIG ConfigVariable : public ConfigVariableBase {
protected:
  INLINE ConfigVariable(const string &name, ValueType type);
  INLINE ConfigVariable(const string &name, ValueType type,
                        const string &description, int flags);

PUBLISHED:
  INLINE ConfigVariable(const string &name);
  INLINE ~ConfigVariable();

  INLINE const ConfigDeclaration *get_default_value() const;

  INLINE const string &get_string_value() const;
  INLINE void set_string_value(const string &value);
  INLINE void clear_value();

  INLINE int get_num_words() const;

  INLINE bool has_string_word(int n) const;
  INLINE bool has_bool_word(int n) const;
  INLINE bool has_int_word(int n) const;
  INLINE bool has_int64_word(int n) const;
  INLINE bool has_double_word(int n) const;

  INLINE string get_string_word(int n) const;
  INLINE bool get_bool_word(int n) const;
  INLINE int get_int_word(int n) const;
  INLINE PN_int64 get_int64_word(int n) const;
  INLINE double get_double_word(int n) const;

  INLINE void set_string_word(int n, const string &value);
  INLINE void set_bool_word(int n, bool value);
  INLINE void set_int_word(int n, int value);
  INLINE void set_int64_word(int n, PN_int64 value);
  INLINE void set_double_word(int n, double value);

protected:
  INLINE bool is_constructed() const;
  void report_unconstructed() const;
};

#include "configVariable.I"

#endif
