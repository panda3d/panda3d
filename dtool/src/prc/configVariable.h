// Filename: configVariable.h
// Created by:  drose (18Oct04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef CONFIGVARIABLE_H
#define CONFIGVARIABLE_H

#include "dtoolbase.h"
#include "configVariableCore.h"
#include "configDeclaration.h"
#include "configVariableManager.h"

////////////////////////////////////////////////////////////////////
//       Class : ConfigVariable
// Description : This is a generic, untyped ConfigVariable.  It is
//               also the base class for the typed ConfigVariables,
//               and contains all of the code common to
//               ConfigVariables of all types.
//
//               Mostly, this class serves as a thin wrapper around
//               ConfigVariableCore and/or ConfigDeclaration, more or
//               less duplicating the interface presented there.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOLCONFIG ConfigVariable {
protected:
  INLINE ConfigVariable(const string &name, 
                        ConfigVariableCore::ValueType type);
  ConfigVariable(const string &name, ConfigVariableCore::ValueType type,
                 int trust_level, const string &description,
                 const string &text);

PUBLISHED:
  INLINE ConfigVariable(const string &name);
  INLINE ~ConfigVariable();

  INLINE const string &get_name() const;

  INLINE ConfigVariableCore::ValueType get_value_type() const;
  INLINE int get_trust_level() const;
  INLINE const string &get_description() const;
  INLINE const string &get_text() const;
  INLINE const ConfigDeclaration *get_default_value() const;

  INLINE string get_string_value() const;
  INLINE void set_string_value(const string &value);

  INLINE bool clear_local_value();
  INLINE bool has_local_value() const;

  INLINE int get_num_words() const;

  INLINE bool has_string_word(int n) const;
  INLINE bool has_bool_word(int n) const;
  INLINE bool has_int_word(int n) const;
  INLINE bool has_double_word(int n) const;

  INLINE string get_string_word(int n) const;
  INLINE bool get_bool_word(int n) const;
  INLINE int get_int_word(int n) const;
  INLINE double get_double_word(int n) const;

  INLINE void set_string_word(int n, const string &value);
  INLINE void set_bool_word(int n, bool value);
  INLINE void set_int_word(int n, int value);
  INLINE void set_double_word(int n, double value);

  INLINE void output(ostream &out) const;
  INLINE void write(ostream &out) const;

protected:
  ConfigVariableCore *_core;
};

INLINE ostream &operator << (ostream &out, const ConfigVariable &variable);

#include "configVariable.I"

#endif
