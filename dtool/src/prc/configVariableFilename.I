// Filename: configVariableFilename.I
// Created by:  drose (22Nov04)
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
//     Function: ConfigVariableFilename::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ConfigVariableFilename::
ConfigVariableFilename(const string &name) :
  ConfigVariable(name, VT_filename)
{
  _core->set_used();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableFilename::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ConfigVariableFilename::
ConfigVariableFilename(const string &name, const Filename &default_value, 
                       const string &description, int flags) :
  ConfigVariable(name, VT_filename, description, flags)
{
  _core->set_default_value(default_value);
  _core->set_used();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableFilename::operator =
//       Access: Published
//  Description: Reassigns the variable's local value.
////////////////////////////////////////////////////////////////////
INLINE void ConfigVariableFilename::
operator = (const Filename &value) {
  set_value(value);
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableFilename::Filename typecast operator
//       Access: Published
//  Description: Returns the variable's value as a Filename.
////////////////////////////////////////////////////////////////////
INLINE ConfigVariableFilename::
operator Filename () const {
  return get_value();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableFilename::empty
//       Access: Published
//  Description: Returns true if the filename is empty, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool ConfigVariableFilename::
empty() const {
  return get_value().empty();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableFilename::Equality operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool ConfigVariableFilename::
operator == (const Filename &other) const {
  return get_value() == other;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableFilename::Inequality operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool ConfigVariableFilename::
operator != (const Filename &other) const {
  return get_value() != other;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableFilename::Ordering operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool ConfigVariableFilename::
operator < (const Filename &other) const {
  return get_value() < other;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableFilename::set_value
//       Access: Published
//  Description: Reassigns the variable's local value.
////////////////////////////////////////////////////////////////////
INLINE void ConfigVariableFilename::
set_value(const Filename &value) {
  set_string_value(value);
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableFilename::get_value
//       Access: Published
//  Description: Returns the variable's value.
////////////////////////////////////////////////////////////////////
INLINE Filename ConfigVariableFilename::
get_value() const {
  return Filename::expand_from(get_string_value());
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableFilename::get_default_value
//       Access: Published
//  Description: Returns the variable's default value.
////////////////////////////////////////////////////////////////////
INLINE Filename ConfigVariableFilename::
get_default_value() const {
  const ConfigDeclaration *decl = ConfigVariable::get_default_value();
  if (decl != (ConfigDeclaration *)NULL) {
    return decl->get_string_word(0);
  }
  return string();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableFilename::get_word
//       Access: Published
//  Description: Returns the variable's nth value.
////////////////////////////////////////////////////////////////////
INLINE Filename ConfigVariableFilename::
get_word(int n) const {
  return Filename::expand_from(get_string_word(n));
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableFilename::set_word
//       Access: Published
//  Description: Reassigns the variable's nth value.  This makes a
//               local copy of the variable's overall value.
////////////////////////////////////////////////////////////////////
INLINE void ConfigVariableFilename::
set_word(int n, const Filename &value) {
  set_string_word(n, value);
}
