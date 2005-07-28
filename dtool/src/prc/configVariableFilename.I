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
  ConfigVariable(name, VT_filename),
  _value_seq(-1),
  _value_stale(true)
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
#ifdef PRC_SAVE_DESCRIPTIONS
  ConfigVariable(name, VT_filename, description, flags),
#else
  ConfigVariable(name, VT_filename, string(), flags),
#endif
  _value_seq(-1),
  _value_stale(true)
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
operator const Filename & () const {
  return get_value();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableFilename::c_str
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const char *ConfigVariableFilename::
c_str() const {
  return get_value().c_str();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableFilename::empty
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool ConfigVariableFilename::
empty() const {
  return get_value().empty();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableFilename::length
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE size_t ConfigVariableFilename::
length() const {
  return get_value().length();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableFilename::Indexing operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE char ConfigVariableFilename::
operator [] (int n) const {
  return get_value()[n];
}


////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableFilename::get_fullpath
//       Access: Public
//  Description: Returns the entire filename: directory, basename,
//               extension.  This is the same thing returned by the
//               string typecast operator, so this function is a
//               little redundant.
////////////////////////////////////////////////////////////////////
INLINE string ConfigVariableFilename::
get_fullpath() const {
  return get_value().get_fullpath();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableFilename::get_dirname
//       Access: Public
//  Description: Returns the directory part of the filename.  This is
//               everything in the filename up to, but not including
//               the rightmost slash.
////////////////////////////////////////////////////////////////////
INLINE string ConfigVariableFilename::
get_dirname() const {
  return get_value().get_dirname();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableFilename::get_basename
//       Access: Public
//  Description: Returns the basename part of the filename.  This is
//               everything in the filename after the rightmost slash,
//               including any extensions.
////////////////////////////////////////////////////////////////////
INLINE string ConfigVariableFilename::
get_basename() const {
  return get_value().get_basename();
}


////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableFilename::get_fullpath_wo_extension
//       Access: Public
//  Description: Returns the full filename--directory and basename
//               parts--except for the extension.
////////////////////////////////////////////////////////////////////
INLINE string ConfigVariableFilename::
get_fullpath_wo_extension() const {
  return get_value().get_fullpath_wo_extension();
}


////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableFilename::get_basename_wo_extension
//       Access: Public
//  Description: Returns the basename part of the filename, without
//               the file extension.
////////////////////////////////////////////////////////////////////
INLINE string ConfigVariableFilename::
get_basename_wo_extension() const {
  return get_value().get_basename_wo_extension();
}


////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableFilename::get_extension
//       Access: Public
//  Description: Returns the file extension.  This is everything after
//               the rightmost dot, if there is one, or the empty
//               string if there is not.
////////////////////////////////////////////////////////////////////
INLINE string ConfigVariableFilename::
get_extension() const {
  return get_value().get_extension();
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
INLINE const Filename &ConfigVariableFilename::
get_value() const {
  if (_value_stale || _value_seq != _core->get_value_seq()) {
    ((ConfigVariableFilename *)this)->reload_value();
  }
  return _value;
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
    return Filename::expand_from(decl->get_string_value());
  }
  return Filename();
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
