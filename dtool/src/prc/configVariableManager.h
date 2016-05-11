/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file configVariableManager.h
 * @author drose
 * @date 2004-10-15
 */

#ifndef CONFIGVARIABLEMANAGER_H
#define CONFIGVARIABLEMANAGER_H

#include "dtoolbase.h"
#include "configFlags.h"
#include "pnotify.h"
#include "globPattern.h"
#include <vector>
#include <map>

class ConfigVariableCore;

/**
 * A global object that maintains the set of ConfigVariables (actually,
 * ConfigVariableCores) everywhere in the world, and keeps them in sorted
 * order.
 */
class EXPCL_DTOOLCONFIG ConfigVariableManager {
protected:
  ConfigVariableManager();
  ~ConfigVariableManager();

PUBLISHED:
  ConfigVariableCore *make_variable(const string &name);
  ConfigVariableCore *make_variable_template(const string &pattern,
                                             ConfigFlags::ValueType type,
                                             const string &default_value,
                                             const string &description = string(),
                                             int flags = 0);


  INLINE size_t get_num_variables() const;
  INLINE ConfigVariableCore *get_variable(size_t n) const;
  MAKE_SEQ(get_variables, get_num_variables, get_variable);
  string get_variable_name(size_t n) const;
  bool is_variable_used(size_t n) const;

  MAKE_SEQ_PROPERTY(variables, get_num_variables, get_variable);

  void output(ostream &out) const;
  void write(ostream &out) const;

  void write_prc_variables(ostream &out) const;

  void list_unused_variables() const;
  void list_variables() const;
  void list_dynamic_variables() const;

  static ConfigVariableManager *get_global_ptr();

private:
  void list_variable(const ConfigVariableCore *variable,
                     bool include_descriptions) const;

  // We have to avoid pmap and pvector, due to the very low-level nature of
  // this stuff.
  typedef vector<ConfigVariableCore *> Variables;
  Variables _variables;

  typedef map<string, ConfigVariableCore *> VariablesByName;
  VariablesByName _variables_by_name;

  typedef map<GlobPattern, ConfigVariableCore *> VariableTemplates;
  VariableTemplates _variable_templates;

  static ConfigVariableManager *_global_ptr;
};

INLINE ostream &operator << (ostream &out, const ConfigVariableManager &variableMgr);

#include "configVariableManager.I"

#endif
