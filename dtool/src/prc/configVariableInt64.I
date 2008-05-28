// Filename: configVariableInt64.I
// Created by:  drose (19Dec07)
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
//     Function: ConfigVariableInt64::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ConfigVariableInt64::
ConfigVariableInt64(const string &name) :
  ConfigVariable(name, VT_int64),
  _local_modified(initial_invalid_cache())
{
  _core->set_used();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableInt64::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ConfigVariableInt64::
ConfigVariableInt64(const string &name, PN_int64 default_value, 
                  const string &description, PN_int64 flags) :
#ifdef PRC_SAVE_DESCRIPTIONS
  ConfigVariable(name, ConfigVariableCore::VT_int64, description, flags),
#else
  ConfigVariable(name, ConfigVariableCore::VT_int64, string(), flags),
#endif
  _local_modified(initial_invalid_cache())
{
  set_default_value(default_value);
  _core->set_used();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableInt64::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ConfigVariableInt64::
ConfigVariableInt64(const string &name, const string &default_value, 
                  const string &description, PN_int64 flags) :
#ifdef PRC_SAVE_DESCRIPTIONS
  ConfigVariable(name, ConfigVariableCore::VT_int64, description, flags),
#else
  ConfigVariable(name, ConfigVariableCore::VT_int64, string(), flags),
#endif
  _local_modified(initial_invalid_cache())
{
  _core->set_default_value(default_value);
  _core->set_used();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableInt64::operator =
//       Access: Published
//  Description: Reassigns the variable's local value.
////////////////////////////////////////////////////////////////////
INLINE void ConfigVariableInt64::
operator = (PN_int64 value) {
  set_value(value);
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableInt64::typecast operator
//       Access: Published
//  Description: Returns the variable's value.
////////////////////////////////////////////////////////////////////
INLINE ConfigVariableInt64::
operator PN_int64 () const {
  return get_value();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableInt64::size()
//       Access: Published
//  Description: Returns the number of unique words in the variable.
////////////////////////////////////////////////////////////////////
INLINE PN_int64 ConfigVariableInt64::
size() const {
  return get_num_words();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableInt64::operator []
//       Access: Published
//  Description: Returns the value of the variable's nth word.
////////////////////////////////////////////////////////////////////
INLINE PN_int64 ConfigVariableInt64::
operator [] (int n) const {
  return get_word(n);
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableInt64::set_value
//       Access: Published
//  Description: Reassigns the variable's local value.
////////////////////////////////////////////////////////////////////
INLINE void ConfigVariableInt64::
set_value(PN_int64 value) {
  set_string_value("");
  set_int64_word(0, value);
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableInt64::get_value
//       Access: Published
//  Description: Returns the variable's value.
////////////////////////////////////////////////////////////////////
INLINE PN_int64 ConfigVariableInt64::
get_value() const {
  TAU_PROFILE("PN_int64 ConfigVariableInt64::get_value() const", " ", TAU_USER);
  if (!is_cache_valid(_local_modified)) {
    mark_cache_valid(((ConfigVariableInt64 *)this)->_local_modified);
    ((ConfigVariableInt64 *)this)->_cache = get_int64_word(0);
  }
  return _cache;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableInt64::get_default_value
//       Access: Published
//  Description: Returns the variable's default value.
////////////////////////////////////////////////////////////////////
INLINE PN_int64 ConfigVariableInt64::
get_default_value() const {
  const ConfigDeclaration *decl = ConfigVariable::get_default_value();
  if (decl != (ConfigDeclaration *)NULL) {
    return decl->get_int64_word(0);
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableInt64::get_word
//       Access: Published
//  Description: Returns the variable's nth value.
////////////////////////////////////////////////////////////////////
INLINE PN_int64 ConfigVariableInt64::
get_word(int n) const {
  return get_int64_word(n);
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableInt64::set_word
//       Access: Published
//  Description: Reassigns the variable's nth value.  This makes a
//               local copy of the variable's overall value.
////////////////////////////////////////////////////////////////////
INLINE void ConfigVariableInt64::
set_word(int n, PN_int64 value) {
  set_int64_word(n, value);
}

