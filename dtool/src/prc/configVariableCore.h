/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file configVariableCore.h
 * @author drose
 * @date 2004-10-15
 */

#ifndef CONFIGVARIABLECORE_H
#define CONFIGVARIABLECORE_H

#include "dtoolbase.h"
#include "configFlags.h"
#include "configPageManager.h"
#include "pnotify.h"

#include <vector>

class ConfigDeclaration;

/**
 * The internal definition of a ConfigVariable.  This object is shared between
 * all instances of a ConfigVariable that use the same variable name.
 *
 * You cannot create a ConfigVariableCore instance directly; instead, use the
 * make() method, which may return a shared instance.  Once created, these
 * objects are never destructed.
 */
class EXPCL_DTOOL_PRC ConfigVariableCore : public ConfigFlags {
private:
  ConfigVariableCore(const std::string &name);
  ConfigVariableCore(const ConfigVariableCore &templ, const std::string &name);
  ~ConfigVariableCore();

PUBLISHED:
  INLINE const std::string &get_name() const;
  INLINE bool is_used() const;

  INLINE ValueType get_value_type() const;
  INLINE const std::string &get_description() const;
  INLINE int get_flags() const;
  INLINE bool is_closed() const;
  INLINE int get_trust_level() const;
  INLINE bool is_dynamic() const;
  INLINE const ConfigDeclaration *get_default_value() const;

  void set_value_type(ValueType value_type);
  void set_flags(int flags);
  void set_description(const std::string &description);
  void set_default_value(const std::string &default_value);
  INLINE void set_used();

  ConfigDeclaration *make_local_value();
  bool clear_local_value();
  INLINE bool has_local_value() const;

  bool has_value() const;
  size_t get_num_declarations() const;
  const ConfigDeclaration *get_declaration(size_t n) const;
  MAKE_SEQ(get_declarations, get_num_declarations, get_declaration);

  INLINE size_t get_num_references() const;
  INLINE const ConfigDeclaration *get_reference(size_t n) const;
  MAKE_SEQ(get_references, get_num_references, get_reference);

  INLINE size_t get_num_trusted_references() const;
  INLINE const ConfigDeclaration *get_trusted_reference(size_t n) const;
  MAKE_SEQ(get_trusted_references, get_num_trusted_references, get_trusted_reference);

  INLINE size_t get_num_unique_references() const;
  INLINE const ConfigDeclaration *get_unique_reference(size_t n) const;
  MAKE_SEQ(get_unique_references, get_num_unique_references, get_unique_reference);
  MAKE_SEQ_PROPERTY(declarations, get_num_declarations, get_declaration);

  void output(std::ostream &out) const;
  void write(std::ostream &out) const;

  MAKE_PROPERTY(name, get_name);
  MAKE_PROPERTY(used, is_used);
  MAKE_PROPERTY(closed, is_closed);
  MAKE_PROPERTY(trust_level, get_trust_level);
  MAKE_PROPERTY(dynamic, is_dynamic);

  MAKE_PROPERTY(value_type, get_value_type, set_value_type);
  MAKE_PROPERTY(description, get_description, set_description);
  MAKE_PROPERTY(default_value, get_default_value, set_default_value);

  MAKE_SEQ_PROPERTY(references, get_num_references, get_reference);
  MAKE_SEQ_PROPERTY(trusted_references, get_num_trusted_references, get_trusted_reference);
  MAKE_SEQ_PROPERTY(unique_references, get_num_unique_references, get_unique_reference);

private:
  void add_declaration(ConfigDeclaration *decl);
  void remove_declaration(ConfigDeclaration *decl);

  INLINE void check_sort_declarations() const;
  void sort_declarations();

private:
  std::string _name;
  bool _is_used;
  ValueType _value_type;
  std::string _description;
  int _flags;
  ConfigDeclaration *_default_value;
  ConfigDeclaration *_local_value;

  typedef std::vector<const ConfigDeclaration *> Declarations;
  Declarations _declarations;
  Declarations _trusted_declarations;
  Declarations _untrusted_declarations;
  Declarations _unique_declarations;
  bool _declarations_sorted;
  bool _value_queried;

  friend class ConfigDeclaration;
  friend class ConfigVariableManager;
};

INLINE std::ostream &operator << (std::ostream &out, const ConfigVariableCore &variable);

#include "configVariableCore.I"

#endif
