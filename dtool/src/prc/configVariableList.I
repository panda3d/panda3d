// Filename: configVariableList.I
// Created by:  drose (20Oct04)
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
//     Function: ConfigVariableList::Destructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ConfigVariableList::
~ConfigVariableList() {
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableList::get_name
//       Access: Published
//  Description: Returns the name of the variable.
////////////////////////////////////////////////////////////////////
INLINE const string &ConfigVariableList::
get_name() const {
  return _core->get_name();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableList::get_value_type
//       Access: Published
//  Description: Returns the stated type of this variable.  This
//               should be VT_list, unless a later variable
//               declaration has changed it.
////////////////////////////////////////////////////////////////////
INLINE ConfigVariableCore::ValueType ConfigVariableList::
get_value_type() const {
  return _core->get_value_type();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableList::get_trust_level
//       Access: Published
//  Description: Returns the minimum trust_level a prc file must
//               demonstrate in order to redefine the value for this
//               variable.  -1 indicates infinite trust: no prc file
//               can redefine it.  Arguably, this should be called the
//               "mistrust level", since the larger the value, the
//               more suspicious we are of prc files.
//
//               This value only has effect in a release build.
////////////////////////////////////////////////////////////////////
INLINE int ConfigVariableList::
get_trust_level() const {
  return _core->get_trust_level();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableList::get_description
//       Access: Published
//  Description: Returns the one-line description of this variable.
//               If the variable has not yet been defined, this will
//               be empty.
////////////////////////////////////////////////////////////////////
INLINE const string &ConfigVariableList::
get_description() const {
  return _core->get_description();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableList::get_text
//       Access: Published
//  Description: Returns the paragraph help text describing the
//               purpose of this variable in greater detail than
//               get_description().  If the variable has not yet been
//               defined, this will be empty.
////////////////////////////////////////////////////////////////////
INLINE const string &ConfigVariableList::
get_text() const {
  return _core->get_text();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableList::get_num_values
//       Access: Published
//  Description: Returns the number of values in the variable.
////////////////////////////////////////////////////////////////////
INLINE int ConfigVariableList::
get_num_values() const {
  return _core->get_num_trusted_references();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableList::get_string_value
//       Access: Published
//  Description: Returns the nth value of the variable.
////////////////////////////////////////////////////////////////////
INLINE string ConfigVariableList::
get_string_value(int n) const {
  const ConfigDeclaration *decl = _core->get_trusted_reference(n);
  if (decl != (ConfigDeclaration *)NULL) {
    return decl->get_string_value();
  }
  return string();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableList::get_num_unique_values
//       Access: Published
//  Description: Returns the number of unique values in the variable.
////////////////////////////////////////////////////////////////////
INLINE int ConfigVariableList::
get_num_unique_values() const {
  return _core->get_num_unique_references();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableList::get_unique_value
//       Access: Published
//  Description: Returns the nth unique value of the variable.
////////////////////////////////////////////////////////////////////
INLINE string ConfigVariableList::
get_unique_value(int n) const {
  const ConfigDeclaration *decl = _core->get_unique_reference(n);
  if (decl != (ConfigDeclaration *)NULL) {
    return decl->get_string_value();
  }
  return string();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableList::size()
//       Access: Published
//  Description: Returns the number of unique values of the variable.
////////////////////////////////////////////////////////////////////
INLINE int ConfigVariableList::
size() const {
  return get_num_unique_values();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableList::operator []
//       Access: Published
//  Description: Returns the nth unique value of the variable.  Note
//               that the indexing operator returns the list of unique
//               values, and so the maximum range is
//               get_num_unique_values().
////////////////////////////////////////////////////////////////////
INLINE string ConfigVariableList::
operator [] (int n) const {
  return get_unique_value(n);
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableList::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ConfigVariableList::
output(ostream &out) const {
  _core->output(out);
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableList::write
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ConfigVariableList::
write(ostream &out) const {
  _core->write(out);
}

INLINE ostream &
operator << (ostream &out, const ConfigVariableList &variable) {
  variable.output(out);
  return out;
}
