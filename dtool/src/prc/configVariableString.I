// Filename: configVariableString.I
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
//     Function: ConfigVariableString::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ConfigVariableString::
ConfigVariableString(const string &name) :
  ConfigVariable(name, ConfigVariableCore::VT_string)
{
  _core->set_used();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableString::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ConfigVariableString::
ConfigVariableString(const string &name, string default_value, int trust_level,
                     const string &description, const string &text) :
  ConfigVariable(name, ConfigVariableCore::VT_string, trust_level, 
                 description, text)
{
  _core->set_default_value(default_value);
  _core->set_used();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableString::operator =
//       Access: Published
//  Description: Reassigns the variable's local value.
////////////////////////////////////////////////////////////////////
INLINE void ConfigVariableString::
operator = (const string &value) {
  set_value(value);
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableString::typecast operator
//       Access: Published
//  Description: Returns the variable's value.
////////////////////////////////////////////////////////////////////
INLINE ConfigVariableString::
operator string () const {
  return get_value();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableString::empty
//       Access: Published
//  Description: Returns true if the string is empty, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool ConfigVariableString::
empty() const {
  return get_value().empty();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableString::Equality operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool ConfigVariableString::
operator == (const string &other) const {
  return get_value() == other;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableString::Inequality operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool ConfigVariableString::
operator != (const string &other) const {
  return get_value() != other;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableString::Ordering operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool ConfigVariableString::
operator < (const string &other) const {
  return get_value() < other;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableString::set_value
//       Access: Published
//  Description: Reassigns the variable's local value.
////////////////////////////////////////////////////////////////////
INLINE void ConfigVariableString::
set_value(const string &value) {
  set_string_value(value);
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableString::get_value
//       Access: Published
//  Description: Returns the variable's value.
////////////////////////////////////////////////////////////////////
INLINE string ConfigVariableString::
get_value() const {
  return get_string_value();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableString::get_default_value
//       Access: Published
//  Description: Returns the variable's default value.
////////////////////////////////////////////////////////////////////
INLINE string ConfigVariableString::
get_default_value() const {
  const ConfigDeclaration *decl = ConfigVariable::get_default_value();
  if (decl != (ConfigDeclaration *)NULL) {
    return decl->get_string_word(0);
  }
  return string();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableString::get_word
//       Access: Published
//  Description: Returns the variable's nth value.
////////////////////////////////////////////////////////////////////
INLINE string ConfigVariableString::
get_word(int n) const {
  return get_string_word(n);
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableString::set_word
//       Access: Published
//  Description: Reassigns the variable's nth value.  This makes a
//               local copy of the variable's overall value.
////////////////////////////////////////////////////////////////////
INLINE void ConfigVariableString::
set_word(int n, const string &value) {
  set_string_word(n, value);
}
