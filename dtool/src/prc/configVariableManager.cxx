// Filename: configVariableManager.cxx
// Created by:  drose (15Oct04)
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

  ConfigVariableCore *variable = new ConfigVariableCore(name);
  _variables_by_name[name] = variable;
  _variables.push_back(variable);
  return variable;
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
  out << "ConfigVariableManager, " << _variables.size() << " variables:\n";
  VariablesByName::const_iterator ni;
  for (ni = _variables_by_name.begin();
       ni != _variables_by_name.end();
       ++ni) {
    ConfigVariableCore *variable = (*ni).second;
    out << "  " << variable->get_name();
    if (!variable->is_used()) {
      out << "  (not used)";
    } else {
      out << " " << variable->get_declaration(0)->get_string_value();
    }
    out << "\n";
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
      cout << variable->get_name() << "\n";
      int num_references = variable->get_num_references();
      for (int i = 0; i < num_references; i++) {
        cout << "  " << variable->get_reference(i)->get_page()->get_name()
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
    ConfigVariableCore *variable = (*ni).second;
    if (variable->is_used()) {
      
      cout << variable->get_name() << " " 
           << variable->get_value_type() << "\n";

      const ConfigDeclaration *decl;

      if (variable->get_value_type() == ConfigVariableCore::VT_list) {
        // We treat a "list" variable as a special case: list all of
        // its values.
        cout << "  current value =\n";
        int num_references = variable->get_num_trusted_references();
        for (int i = 0; i < num_references; i++) {
          decl = variable->get_trusted_reference(i);
          cout << "    " << decl->get_string_value()
               << "  (from " << decl->get_page()->get_name() << ")\n";
        }
        
      } else {
        // An ordinary, non-list variable gets one line for its
        // current value (if it has one) and another line for its
        // default value.
        decl = variable->get_declaration(0);
        if (decl != variable->get_default_value()) {
          cout << "  current value = " << decl->get_string_value();
          if (!decl->get_page()->is_special()) {
            cout << "  (from " << decl->get_page()->get_name() << ")\n";
          } else {
            cout << "  (defined locally)\n";
          }
        }

        decl = variable->get_default_value();
        if (decl != (ConfigDeclaration *)NULL) {
          cout << "  default value = " << decl->get_string_value() << "\n";
        }
      }

      if (!variable->get_description().empty()) {
        cout << "  " << variable->get_description() << "\n";
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableManager::describe_variables
//       Access: Published
//  Description: Writes a list of all the variables along with each
//               variable's long description.
////////////////////////////////////////////////////////////////////
void ConfigVariableManager::
describe_variables() const {
  VariablesByName::const_iterator ni;
  for (ni = _variables_by_name.begin();
       ni != _variables_by_name.end();
       ++ni) {
    ConfigVariableCore *variable = (*ni).second;
    if (variable->is_used()) {
      cout << variable->get_name() << " "
           << variable->get_value_type() << "\n";

      const ConfigDeclaration *decl = variable->get_default_value();
      if (decl != (ConfigDeclaration *)NULL) {
        cout << "  default value = " << decl->get_string_value() << "\n";
      }

      if (!variable->get_text().empty()) {
        cout << "  " << variable->get_text() << "\n";
      } else if (!variable->get_description().empty()) {
        cout << "  " << variable->get_description() << "\n";
      }
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
