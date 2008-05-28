// Filename: configVariableManager.cxx
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

#include "configVariableManager.h"
#include "configVariableCore.h"
#include "configDeclaration.h"
#include "configPage.h"
#include "config_prc.h"

ConfigVariableManager *ConfigVariableManager::_global_ptr = NULL;

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableManager::Constructor
//       Access: Protected
//  Description: The constructor is private (actually, just protected,
//               but only to avoid a gcc compiler warning) because it
//               should not be explicitly constructed.  There is only
//               one ConfigVariableManager, and it constructs
//               itself.
////////////////////////////////////////////////////////////////////
ConfigVariableManager::
ConfigVariableManager() {
  init_memory_hook();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableManager::Destructor
//       Access: Protected
//  Description: The ConfigVariableManager destructor should never be
//               called, because this is a global object that is never
//               freed.
////////////////////////////////////////////////////////////////////
ConfigVariableManager::
~ConfigVariableManager() {
  prc_cat->error()
    << "Internal error--ConfigVariableManager destructor called!\n";
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableManager::make_variable
//       Access: Published
//  Description: Creates and returns a new, undefined
//               ConfigVariableCore with the indicated name; or if a
//               variable with this name has already been created,
//               returns that one instead.
////////////////////////////////////////////////////////////////////
ConfigVariableCore *ConfigVariableManager::
make_variable(const string &name) {
  VariablesByName::iterator ni;
  ni = _variables_by_name.find(name);
  if (ni != _variables_by_name.end()) {
    return (*ni).second;
  }

  ConfigVariableCore *variable = NULL;

  // See if there's a template that matches this name.
  VariableTemplates::const_iterator ti;
  for (ti = _variable_templates.begin();
       ti != _variable_templates.end() && variable == (ConfigVariableCore *)NULL;
       ++ti) {
    const GlobPattern &pattern = (*ti).first;
    ConfigVariableCore *templ = (*ti).second;
    if (pattern.matches(name)) {
      variable = new ConfigVariableCore(*templ, name);
    }
  }

  if (variable == (ConfigVariableCore *)NULL) {
    variable = new ConfigVariableCore(name);
  }

  _variables_by_name[name] = variable;
  _variables.push_back(variable);

  return variable;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableManager::make_variable_template
//       Access: Published
//  Description: Defines a variable "template" to match against
//               dynamically-defined variables that may or may not be
//               created in the future.
//
//               The template consists of a glob pattern,
//               e.g. "notify-level-*", which will be tested against
//               any config variable passed to a future call to
//               make_variable().  If the pattern matches, the
//               returned ConfigVariableCore is copied to define the
//               new variable, instead of creating a default, empty
//               one.
//
//               This is useful to pre-specify default values for a
//               family of variables that all have similar properties,
//               and all may not be created at the same time.  It is
//               especially useful to avoid cluttering up the list of
//               available variables with user-declared variables that
//               have not been defined yet by the application
//               (e.g. "egg-object-type-*").
//
//               This method basically pre-defines all variables that
//               match the specified glob pattern.
////////////////////////////////////////////////////////////////////
ConfigVariableCore *ConfigVariableManager::
make_variable_template(const string &pattern, 
                       ConfigFlags::ValueType value_type,
                       const string &default_value,
                       const string &description, int flags) {
  ConfigVariableCore *core;

  GlobPattern gp(pattern);
  VariableTemplates::const_iterator ti = _variable_templates.find(gp);
  if (ti != _variable_templates.end()) {
    core = (*ti).second;

  } else {
    core = new ConfigVariableCore(pattern);
    _variable_templates[gp] = core;
  }

  if (value_type != ConfigFlags::VT_undefined) {
    core->set_value_type(value_type);
  }
  if (!default_value.empty() || 
      core->get_default_value() == (ConfigDeclaration *)NULL) {
    core->set_default_value(default_value);
  }
  if (!description.empty()) {
    core->set_description(description);
  }
  if (flags != 0) {
    core->set_flags(flags);
  }
  core->set_used();

  // Also apply the same changes to any previously-defined variables
  // that match the pattern.
  Variables::iterator vi;
  for (vi = _variables.begin(); vi != _variables.end(); ++vi) {
    ConfigVariableCore *variable = (*vi);
    if (gp.matches(variable->get_name())) {
      if (value_type != ConfigFlags::VT_undefined) {
        variable->set_value_type(value_type);
      }
      if (!default_value.empty() || 
          variable->get_default_value() == (ConfigDeclaration *)NULL) {
        variable->set_default_value(default_value);
      }
      if (!description.empty()) {
        variable->set_description(description);
      }
      if (flags != 0) {
        variable->set_flags(flags);
      }
      variable->set_used();
    }
  }

  return core;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableManager::get_variable_name
//       Access: Published
//  Description: Returns the name of the nth active ConfigVariable in
//               the list.
////////////////////////////////////////////////////////////////////
string ConfigVariableManager::
get_variable_name(int n) const {
  if (n >= 0 && n < (int)_variables.size()) {
    return _variables[n]->get_name();
  }
  return string();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableManager::is_variable_used
//       Access: Published
//  Description: Returns true if the nth active ConfigVariable in
//               the list has been used by code, false otherwise.
////////////////////////////////////////////////////////////////////
bool ConfigVariableManager::
is_variable_used(int n) const {
  if (n >= 0 && n < (int)_variables.size()) {
    return _variables[n]->is_used();
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableManager::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void ConfigVariableManager::
output(ostream &out) const {
  out << "ConfigVariableManager, " << _variables.size() << " variables.";
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableManager::write
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void ConfigVariableManager::
write(ostream &out) const {
  VariablesByName::const_iterator ni;
  for (ni = _variables_by_name.begin();
       ni != _variables_by_name.end();
       ++ni) {
    ConfigVariableCore *variable = (*ni).second;
    if (variable->get_num_trusted_references() != 0 ||
        variable->has_local_value()) {
      list_variable(variable, false);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableManager::write_prc_variables
//       Access: Published
//  Description: Writes all of the prc-set config variables, as they
//               appear in a prc file somewhere, one per line, very
//               concisely.  This lists the dominant value in the prc
//               file; it does not list shadowed values, and it does
//               not list locally-set values.
//
//               This is mainly intended for generating a hash of the
//               input config file state.
////////////////////////////////////////////////////////////////////
void ConfigVariableManager::
write_prc_variables(ostream &out) const {
  VariablesByName::const_iterator ni;
  for (ni = _variables_by_name.begin();
       ni != _variables_by_name.end();
       ++ni) {
    ConfigVariableCore *variable = (*ni).second;
    if (variable->get_num_trusted_references() != 0) {
      if (variable->get_value_type() == ConfigVariableCore::VT_list ||
          variable->get_value_type() == ConfigVariableCore::VT_search_path) {
        // List all of the values for a "list" variable.
        int num_references = variable->get_num_trusted_references();
        for (int i = 0; i < num_references; i++) {
          out << variable->get_name() << " " 
              << variable->get_trusted_reference(i)->get_string_value()
              << "\n";
        }
      } else {
        // List just the one value for a non-list variable.
        out << variable->get_name() << " " 
            << variable->get_trusted_reference(0)->get_string_value()
            << "\n";
      }
    }
  }
}
  
////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableManager::list_unused_variables
//       Access: Published
//  Description: Writes a list of all the variables that have been
//               defined in a prc file without having been declared
//               somewhere in code.
////////////////////////////////////////////////////////////////////
void ConfigVariableManager::
list_unused_variables() const {
  VariablesByName::const_iterator ni;
  for (ni = _variables_by_name.begin();
       ni != _variables_by_name.end();
       ++ni) {
    ConfigVariableCore *variable = (*ni).second;
    if (!variable->is_used()) {
      nout << variable->get_name() << "\n";
      int num_references = variable->get_num_references();
      for (int i = 0; i < num_references; i++) {
        nout << "  " << variable->get_reference(i)->get_page()->get_name()
             << "\n";
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableManager::list_variables
//       Access: Published
//  Description: Writes a list of all the variables that have been
//               declared somewhere in code, along with a brief
//               description.
////////////////////////////////////////////////////////////////////
void ConfigVariableManager::
list_variables() const {
  VariablesByName::const_iterator ni;
  for (ni = _variables_by_name.begin();
       ni != _variables_by_name.end();
       ++ni) {
    const ConfigVariableCore *variable = (*ni).second;
    if (variable->is_used() && !variable->is_dynamic()) {
      list_variable(variable, true);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableManager::list_dynamic_variables
//       Access: Published
//  Description: Writes a list of all the "dynamic" variables that
//               have been declared somewhere in code, along with a
//               brief description.  This is a (usually large) list of
//               config variables that are declared with a generated
//               variable name.
////////////////////////////////////////////////////////////////////
void ConfigVariableManager::
list_dynamic_variables() const {
  VariablesByName::const_iterator ni;
  for (ni = _variables_by_name.begin();
       ni != _variables_by_name.end();
       ++ni) {
    const ConfigVariableCore *variable = (*ni).second;
    if (variable->is_used() && variable->is_dynamic()) {
      list_variable(variable, false);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableManager::get_global_ptr
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
ConfigVariableManager *ConfigVariableManager::
get_global_ptr() {
  if (_global_ptr == (ConfigVariableManager *)NULL) {
    _global_ptr = new ConfigVariableManager;
  }
  return _global_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableManager::list_variable
//       Access: Private
//  Description: Lists a single variable and its value.
////////////////////////////////////////////////////////////////////
void ConfigVariableManager::
list_variable(const ConfigVariableCore *variable, 
              bool include_descriptions) const {
  if (!variable->is_used()) {
    // If the variable is unused, just show its name.
    nout << variable->get_name() << "  not used";
    if (variable->get_num_references() > 0) {
      nout << " (referenced in " 
           << variable->get_reference(0)->get_page()->get_name()
           << ")";
    }
    nout << "\n";
  
  } else {
    // If the variable is used--it's been defined somewhere--show its
    // name, its type, its current and default values, and if
    // available, its description.

    nout << variable->get_name() << " " 
         << variable->get_value_type() << "\n";
    
    const ConfigDeclaration *decl;
    
    if (variable->get_value_type() == ConfigVariableCore::VT_list ||
        variable->get_value_type() == ConfigVariableCore::VT_search_path) {
      // We treat a "list" variable as a special case: list all of
      // its values.
      nout << "  current value:\n";
      int num_references = variable->get_num_trusted_references();
      for (int i = 0; i < num_references; i++) {
        decl = variable->get_trusted_reference(i);
        nout << "    " << decl->get_string_value()
             << "  (from " << decl->get_page()->get_name() << ")\n";
      }
      
    } else {
      // An ordinary, non-list variable gets one line for its
      // current value (if it has one) and another line for its
      // default value.
      decl = variable->get_declaration(0);
      if (decl != variable->get_default_value()) {
        nout << "  current value = " << decl->get_string_value();
        if (!decl->get_page()->is_special()) {
          nout << "  (from " << decl->get_page()->get_name() << ")\n";
        } else {
          nout << "  (defined locally)\n";
        }
      }
      
      decl = variable->get_default_value();
      if (decl != (ConfigDeclaration *)NULL) {
        nout << "  default value = " << decl->get_string_value() << "\n";
      }
    }
    
    if (!variable->get_description().empty() && include_descriptions) {
      nout << "  " << variable->get_description() << "\n";
    }
  }

  nout << "\n";
}
