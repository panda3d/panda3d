// Filename: configVariableCore.h
// Created by:  drose (15Oct04)
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

#ifndef CONFIGVARIABLECORE_H
#define CONFIGVARIABLECORE_H

#include "dtoolbase.h"
#include "configFlags.h"
#include "configPageManager.h"
#include "pnotify.h"

#include <vector>

class ConfigDeclaration;

////////////////////////////////////////////////////////////////////
//       Class : ConfigVariableCore
// Description : The internal definition of a ConfigVariable.  This
//               object is shared between all instances of a
//               ConfigVariable that use the same variable name.
//
//               You cannot create a ConfigVariableCore instance
//               directly; instead, use the make() method, which may
//               return a shared instance.  Once created, these
//               objects are never destructed.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOLCONFIG ConfigVariableCore : public ConfigFlags {
private:
  ConfigVariableCore(const string &name);
  ConfigVariableCore(const ConfigVariableCore &templ, const string &name);
  ~ConfigVariableCore();

PUBLISHED:
  INLINE const string &get_name() const;
  INLINE bool is_used() const;

  INLINE ValueType get_value_type() const;
  INLINE const string &get_description() const;
  INLINE int get_flags() const;
  INLINE bool is_closed() const;
  INLINE int get_trust_level() const;
  INLINE bool is_dynamic() const;
  INLINE const ConfigDeclaration *get_default_value() const;

  void set_value_type(ValueType value_type);
  void set_flags(int flags);
  void set_description(const string &description);
  void set_default_value(const string &default_value);
  INLINE void set_used();

  ConfigDeclaration *make_local_value();
  bool clear_local_value();
  INLINE bool has_local_value() const;

  bool has_value() const;
  int get_num_declarations() const;
  const ConfigDeclaration *get_declaration(int n) const;
  MAKE_SEQ(get_declarations, get_num_declarations, get_declaration);

  INLINE int get_num_references() const;
  INLINE const ConfigDeclaration *get_reference(int n) const;
  MAKE_SEQ(get_references, get_num_references, get_reference);

  INLINE int get_num_trusted_references() const;
  INLINE const ConfigDeclaration *get_trusted_reference(int n) const;
  MAKE_SEQ(get_trusted_references, get_num_trusted_references, get_trusted_reference);

  INLINE int get_num_unique_references() const;
  INLINE const ConfigDeclaration *get_unique_reference(int n) const;
  MAKE_SEQ(get_unique_references, get_num_unique_references, get_unique_reference);

  void output(ostream &out) const;
  void write(ostream &out) const;

private:
  void add_declaration(ConfigDeclaration *decl);
  void remove_declaration(ConfigDeclaration *decl);

  INLINE void check_sort_declarations() const;
  void sort_declarations();

private:
  string _name;
  bool _is_used;
  ValueType _value_type;
  string _description;
  int _flags;
  ConfigDeclaration *_default_value;
  ConfigDeclaration *_local_value;

  typedef vector<const ConfigDeclaration *> Declarations;
  Declarations _declarations;
  Declarations _trusted_declarations;
  Declarations _untrusted_declarations;
  Declarations _unique_declarations;
  bool _declarations_sorted;
  bool _value_queried;

  friend class ConfigDeclaration;
  friend class ConfigVariableManager;
};

INLINE ostream &operator << (ostream &out, const ConfigVariableCore &variable);

#include "configVariableCore.I"

#endif
