// Filename: configVariableDouble.I
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
//     Function: ConfigVariableDouble::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ConfigVariableDouble::
ConfigVariableDouble(const string &name) :
  ConfigVariable(name, VT_double)
{
  _core->set_used();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableDouble::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ConfigVariableDouble::
ConfigVariableDouble(const string &name, double default_value, 
                     const string &description, int flags) :
#ifdef PRC_SAVE_DESCRIPTIONS
  ConfigVariable(name, ConfigVariableCore::VT_double, description, flags)
#else
  ConfigVariable(name, ConfigVariableCore::VT_double, string(), flags)
#endif
{
  set_default_value(default_value);
  _core->set_used();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableDouble::operator =
//       Access: Published
//  Description: Reassigns the variable's local value.
////////////////////////////////////////////////////////////////////
INLINE void ConfigVariableDouble::
operator = (double value) {
  set_value(value);
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableDouble::typecast operator
//       Access: Published
//  Description: Returns the variable's value.
////////////////////////////////////////////////////////////////////
INLINE ConfigVariableDouble::
operator double () const {
  return get_value();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableDouble::size()
//       Access: Published
//  Description: Returns the number of unique words in the variable.
////////////////////////////////////////////////////////////////////
INLINE int ConfigVariableDouble::
size() const {
  return get_num_words();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableDouble::operator []
//       Access: Published
//  Description: Returns the value of the variable's nth word.
////////////////////////////////////////////////////////////////////
INLINE double ConfigVariableDouble::
operator [] (int n) const {
  return get_word(n);
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableDouble::set_value
//       Access: Published
//  Description: Reassigns the variable's local value.
////////////////////////////////////////////////////////////////////
INLINE void ConfigVariableDouble::
set_value(double value) {
  set_string_value("");
  set_double_word(0, value);
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableDouble::get_value
//       Access: Published
//  Description: Returns the variable's value.
////////////////////////////////////////////////////////////////////
INLINE double ConfigVariableDouble::
get_value() const {
  return get_double_word(0);
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableDouble::get_default_value
//       Access: Published
//  Description: Returns the variable's default value.
////////////////////////////////////////////////////////////////////
INLINE double ConfigVariableDouble::
get_default_value() const {
  const ConfigDeclaration *decl = ConfigVariable::get_default_value();
  if (decl != (ConfigDeclaration *)NULL) {
    return decl->get_double_word(0);
  }
  return 0.0;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableDouble::get_word
//       Access: Published
//  Description: Returns the variable's nth value.
////////////////////////////////////////////////////////////////////
INLINE double ConfigVariableDouble::
get_word(int n) const {
  return get_double_word(n);
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableDouble::set_word
//       Access: Published
//  Description: Reassigns the variable's nth value.  This makes a
//               local copy of the variable's overall value.
////////////////////////////////////////////////////////////////////
INLINE void ConfigVariableDouble::
set_word(int n, double value) {
  set_double_word(n, value);
}
