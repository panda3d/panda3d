// Filename: configVariableCore.I
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


////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableCore::get_name
//       Access: Public
//  Description: Returns the name of the variable.
////////////////////////////////////////////////////////////////////
INLINE const string &ConfigVariableCore::
get_name() const {
  return _name;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableCore::is_used
//       Access: Public
//  Description: Returns true if the variable has been referenced by a
//               ConfigVariable somewhere in code, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool ConfigVariableCore::
is_used() const {
  return _is_used;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableCore::get_value_type
//       Access: Public
//  Description: Returns the stated type of this variable.  If the
//               variable has not yet been defined, this will be
//               VT_undefined.
////////////////////////////////////////////////////////////////////
INLINE ConfigVariableCore::ValueType ConfigVariableCore::
get_value_type() const {
  return _value_type;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableCore::get_trust_level
//       Access: Public
//  Description: Returns the minimum trust_level a prc file must
//               demonstrate in order to redefine the value for this
//               variable.  -1 indicates infinite trust: no prc file
//               can redefine it.  Arguably, this should be called the
//               "mistrust level", since the larger the value, the
//               more suspicious we are of prc files.
//
//               This value only has effect in a release build.
////////////////////////////////////////////////////////////////////
INLINE int ConfigVariableCore::
get_trust_level() const {
  return _trust_level;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableCore::get_description
//       Access: Public
//  Description: Returns the one-line description of this variable.
//               If the variable has not yet been defined, this will
//               be empty.
////////////////////////////////////////////////////////////////////
INLINE const string &ConfigVariableCore::
get_description() const {
  return _description;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableCore::get_text
//       Access: Public
//  Description: Returns the paragraph help text describing the
//               purpose of this variable in greater detail than
//               get_description().  If the variable has not yet been
//               defined, this will be empty.
////////////////////////////////////////////////////////////////////
INLINE const string &ConfigVariableCore::
get_text() const {
  return _text;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableCore::get_default_value
//       Access: Public
//  Description: Returns the default variable specified for this
//               variable.  If the variable has not yet been defined,
//               this will return NULL.
////////////////////////////////////////////////////////////////////
INLINE const ConfigDeclaration *ConfigVariableCore::
get_default_value() const {
  return _default_value;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableCore::set_used
//       Access: Public
//  Description: Marks that the variable has been "declared" by a
//               ConfigVariable.
////////////////////////////////////////////////////////////////////
INLINE void ConfigVariableCore::
set_used() {
  _is_used = true;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableCore::has_local_value
//       Access: Public
//  Description: Returns true if this variable's value has been
//               shadowed by a local assignment (as created via
//               make_local_value()), or false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool ConfigVariableCore::
has_local_value() const {
  return _local_value != (ConfigDeclaration *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableCore::get_num_references
//       Access: Public
//  Description: Returns the number of prc files that reference this
//               variable.  This is not exactly the same as the number
//               of declarations; see get_reference().
////////////////////////////////////////////////////////////////////
INLINE int ConfigVariableCore::
get_num_references() const {
  check_sort_declarations();
  return _declarations.size();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableCore::get_reference
//       Access: Public
//  Description: Returns the nth declaration in a prc file that
//               references this variable.  This is similar, but not
//               identical to, get_declaration().  The difference is
//               that this will list *only* true references in a prc
//               file, and will not list default values or
//               locally-assigned values; it also will list even the
//               untrusted files.
////////////////////////////////////////////////////////////////////
INLINE const ConfigDeclaration *ConfigVariableCore::
get_reference(int n) const {
  check_sort_declarations();
  if (n >= 0 && n < (int)_declarations.size()) {
    return _declarations[n];
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableCore::get_num_trusted_references
//       Access: Public
//  Description: Returns the number of trusted prc files that
//               reference this variable.  See also
//               get_num_references().
////////////////////////////////////////////////////////////////////
INLINE int ConfigVariableCore::
get_num_trusted_references() const {
  check_sort_declarations();
  return _trusted_declarations.size();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableCore::get_trusted_reference
//       Access: Public
//  Description: Returns the nth declaration in a trusted prc file
//               that references this variable.  This is similar, but
//               not identical to, get_declaration().  The difference
//               is that this will list *only* true references in a
//               prc file, and will not list default values or
//               locally-assigned values.
//
//               This is also similar to get_reference(), except that
//               it only lists the trusted declarations, omitting the
//               untrusted ones.
////////////////////////////////////////////////////////////////////
INLINE const ConfigDeclaration *ConfigVariableCore::
get_trusted_reference(int n) const {
  check_sort_declarations();
  if (n >= 0 && n < (int)_trusted_declarations.size()) {
    return _trusted_declarations[n];
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableCore::get_num_unique_references
//       Access: Public
//  Description: Returns the number of trusted, unique (by string
//               value) values there exist for this variable.
////////////////////////////////////////////////////////////////////
INLINE int ConfigVariableCore::
get_num_unique_references() const {
  check_sort_declarations();
  return _unique_declarations.size();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableCore::get_unique_reference
//       Access: Public
//  Description: Returns the nth trusted, unique value for this
//               variable.  This is similar to
//               get_trusted_reference(), except that duplicate values
//               are removed.
////////////////////////////////////////////////////////////////////
INLINE const ConfigDeclaration *ConfigVariableCore::
get_unique_reference(int n) const {
  check_sort_declarations();
  if (n >= 0 && n < (int)_unique_declarations.size()) {
    return _unique_declarations[n];
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableCore::check_sort_declarations()
//       Access: Private
//  Description: Called internally to ensure that the list of
//               declarations is properly sorted.
////////////////////////////////////////////////////////////////////
INLINE void ConfigVariableCore::
check_sort_declarations() const {
  // First, make sure that all of the implicit .prc files have been
  // loaded.  This may unsort the list by adding a bunch more
  // declarations.
  ConfigPageManager::get_global_ptr()->load_implicit_pages();
  
  // Then sort the list if it needs it.
  if (!_declarations_sorted) {
    ((ConfigVariableCore *)this)->sort_declarations();
  }
}

INLINE ostream &
operator << (ostream &out, const ConfigVariableCore &variable) {
  variable.output(out);
  return out;
}
