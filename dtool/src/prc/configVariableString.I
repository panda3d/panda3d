// Filename: configVariableString.I
// Created by:  drose (20Oct04)
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


////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableString::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ConfigVariableString::
ConfigVariableString(const string &name) :
  ConfigVariable(name, VT_string),
  _local_modified(initial_invalid_cache())
{
  _core->set_used();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableString::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ConfigVariableString::
ConfigVariableString(const string &name, const string &default_value, 
                     const string &description, int flags) :
#ifdef PRC_SAVE_DESCRIPTIONS
  ConfigVariable(name, VT_string, description, flags),
#else
  ConfigVariable(name, VT_string, string(), flags),
#endif
  _local_modified(initial_invalid_cache())
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
//     Function: ConfigVariableString::string typecast operator
//       Access: Published
//  Description: Returns the variable's value.
////////////////////////////////////////////////////////////////////
INLINE ConfigVariableString::
operator const string & () const {
  return get_value();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableString::c_str
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const char *ConfigVariableString::
c_str() const {
  return get_value().c_str();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableString::empty
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool ConfigVariableString::
empty() const {
  return get_value().empty();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableString::length
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE size_t ConfigVariableString::
length() const {
  return get_value().length();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableString::Indexing operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE char ConfigVariableString::
operator [] (int n) const {
  assert(n >= 0 && n < (int)length());
  return get_value()[n];
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
INLINE const string &ConfigVariableString::
get_value() const {
  TAU_PROFILE("const string &ConfigVariableString::get_value() const", " ", TAU_USER);
  if (!is_cache_valid(_local_modified)) {
    mark_cache_valid(((ConfigVariableString *)this)->_local_modified);
    ((ConfigVariableString *)this)->_cache = get_string_value();
  }
  return _cache;
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
    return decl->get_string_value();
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
