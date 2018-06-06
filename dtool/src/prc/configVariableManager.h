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
class EXPCL_DTOOL_PRC ConfigVariableManager {
protected:
  ConfigVariableManager();
  ~ConfigVariableManager();

PUBLISHED:
  ConfigVariableCore *make_variable(const std::string &name);
  ConfigVariableCore *make_variable_template(const std::string &pattern,
                                             ConfigFlags::ValueType type,
                                             const std::string &default_value,
                                             const std::string &description = std::string(),
                                             int flags = 0);


  INLINE size_t get_num_variables() const;
  INLINE ConfigVariableCore *get_variable(size_t n) const;
  MAKE_SEQ(get_variables, get_num_variables, get_variable);
  std::string get_variable_name(size_t n) const;
  bool is_variable_used(size_t n) const;

  MAKE_SEQ_PROPERTY(variables, get_num_variables, get_variable);

  void output(std::ostream &out) const;
  void write(std::ostream &out) const;

  void write_prc_variables(std::ostream &out) const;

  void list_unused_variables() const;
  void list_variables() const;
  void list_dynamic_variables() const;

  static ConfigVariableManager *get_global_ptr();

private:
  void list_variable(const ConfigVariableCore *variable,
                     bool include_descriptions) const;

  // We have to avoid pmap and pvector, due to the very low-level nature of
  // this stuff.
  typedef std::vector<ConfigVariableCore *> Variables;
  Variables _variables;

  typedef std::map<std::string, ConfigVariableCore *> VariablesByName;
  VariablesByName _variables_by_name;

  typedef std::map<GlobPattern, ConfigVariableCore *> VariableTemplates;
  VariableTemplates _variable_templates;

  static ConfigVariableManager *_global_ptr;
};

INLINE std::ostream &operator << (std::ostream &out, const ConfigVariableManager &variableMgr);

#include "configVariableManager.I"

#endif
