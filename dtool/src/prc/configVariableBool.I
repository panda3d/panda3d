// Filename: configVariableBool.I
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
//     Function: ConfigVariableBool::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ConfigVariableBool::
ConfigVariableBool(const string &name) :
  ConfigVariable(name, VT_bool)
{
  _core->set_used();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableBool::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ConfigVariableBool::
ConfigVariableBool(const string &name, bool default_value, 
                   const string &description, int flags) :
  ConfigVariable(name, VT_bool, description, flags)
{
  _core->set_default_value(default_value ? "1" : "0");
  _core->set_used();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableBool::operator =
//       Access: Published
//  Description: Reassigns the variable's local value.
////////////////////////////////////////////////////////////////////
INLINE void ConfigVariableBool::
operator = (bool value) {
  set_value(value);
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableBool::typecast operator
//       Access: Published
//  Description: Returns the variable's value.
////////////////////////////////////////////////////////////////////
INLINE ConfigVariableBool::
operator bool () const {
  return get_value();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableBool::size()
//       Access: Published
//  Description: Returns the number of unique words in the variable.
////////////////////////////////////////////////////////////////////
INLINE int ConfigVariableBool::
size() const {
  return get_num_words();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableBool::operator []
//       Access: Published
//  Description: Returns the value of the variable's nth word.
////////////////////////////////////////////////////////////////////
INLINE bool ConfigVariableBool::
operator [] (int n) const {
  return get_word(n);
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableBool::set_value
//       Access: Published
//  Description: Reassigns the variable's local value.
////////////////////////////////////////////////////////////////////
INLINE void ConfigVariableBool::
set_value(bool value) {
  set_string_value("");
  set_bool_word(0, value);
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableBool::get_value
//       Access: Published
//  Description: Returns the variable's value.
////////////////////////////////////////////////////////////////////
INLINE bool ConfigVariableBool::
get_value() const {
  return get_bool_word(0);
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableBool::get_default_value
//       Access: Published
//  Description: Returns the variable's default value.
////////////////////////////////////////////////////////////////////
INLINE bool ConfigVariableBool::
get_default_value() const {
  const ConfigDeclaration *decl = ConfigVariable::get_default_value();
  if (decl != (ConfigDeclaration *)NULL) {
    return decl->get_bool_word(0);
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableBool::get_word
//       Access: Published
//  Description: Returns the variable's nth value.
////////////////////////////////////////////////////////////////////
INLINE bool ConfigVariableBool::
get_word(int n) const {
  return get_bool_word(n);
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableBool::set_word
//       Access: Published
//  Description: Reassigns the variable's nth value.  This makes a
//               local copy of the variable's overall value.
////////////////////////////////////////////////////////////////////
INLINE void ConfigVariableBool::
set_word(int n, bool value) {
  set_bool_word(n, value);
}
