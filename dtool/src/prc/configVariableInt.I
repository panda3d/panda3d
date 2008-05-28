// Filename: configVariableInt.I
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
//     Function: ConfigVariableInt::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ConfigVariableInt::
ConfigVariableInt(const string &name) :
  ConfigVariable(name, VT_int),
  _local_modified(initial_invalid_cache())
{
  _core->set_used();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableInt::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ConfigVariableInt::
ConfigVariableInt(const string &name, int default_value, 
                  const string &description, int flags) :
#ifdef PRC_SAVE_DESCRIPTIONS
  ConfigVariable(name, ConfigVariableCore::VT_int, description, flags),
#else
  ConfigVariable(name, ConfigVariableCore::VT_int, string(), flags),
#endif
  _local_modified(initial_invalid_cache())
{
  set_default_value(default_value);
  _core->set_used();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableInt::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ConfigVariableInt::
ConfigVariableInt(const string &name, const string &default_value, 
                  const string &description, int flags) :
#ifdef PRC_SAVE_DESCRIPTIONS
  ConfigVariable(name, ConfigVariableCore::VT_int, description, flags),
#else
  ConfigVariable(name, ConfigVariableCore::VT_int, string(), flags),
#endif
  _local_modified(initial_invalid_cache())
{
  _core->set_default_value(default_value);
  _core->set_used();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableInt::operator =
//       Access: Published
//  Description: Reassigns the variable's local value.
////////////////////////////////////////////////////////////////////
INLINE void ConfigVariableInt::
operator = (int value) {
  set_value(value);
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableInt::typecast operator
//       Access: Published
//  Description: Returns the variable's value.
////////////////////////////////////////////////////////////////////
INLINE ConfigVariableInt::
operator int () const {
  return get_value();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableInt::size()
//       Access: Published
//  Description: Returns the number of unique words in the variable.
////////////////////////////////////////////////////////////////////
INLINE int ConfigVariableInt::
size() const {
  return get_num_words();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableInt::operator []
//       Access: Published
//  Description: Returns the value of the variable's nth word.
////////////////////////////////////////////////////////////////////
INLINE int ConfigVariableInt::
operator [] (int n) const {
  return get_word(n);
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableInt::set_value
//       Access: Published
//  Description: Reassigns the variable's local value.
////////////////////////////////////////////////////////////////////
INLINE void ConfigVariableInt::
set_value(int value) {
  set_string_value("");
  set_int_word(0, value);
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableInt::get_value
//       Access: Published
//  Description: Returns the variable's value.
////////////////////////////////////////////////////////////////////
INLINE int ConfigVariableInt::
get_value() const {
  TAU_PROFILE("int ConfigVariableInt::get_value() const", " ", TAU_USER);
  if (!is_cache_valid(_local_modified)) {
    mark_cache_valid(((ConfigVariableInt *)this)->_local_modified);
    ((ConfigVariableInt *)this)->_cache = get_int_word(0);
  }
  return _cache;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableInt::get_default_value
//       Access: Published
//  Description: Returns the variable's default value.
////////////////////////////////////////////////////////////////////
INLINE int ConfigVariableInt::
get_default_value() const {
  const ConfigDeclaration *decl = ConfigVariable::get_default_value();
  if (decl != (ConfigDeclaration *)NULL) {
    return decl->get_int_word(0);
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableInt::get_word
//       Access: Published
//  Description: Returns the variable's nth value.
////////////////////////////////////////////////////////////////////
INLINE int ConfigVariableInt::
get_word(int n) const {
  return get_int_word(n);
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableInt::set_word
//       Access: Published
//  Description: Reassigns the variable's nth value.  This makes a
//               local copy of the variable's overall value.
////////////////////////////////////////////////////////////////////
INLINE void ConfigVariableInt::
set_word(int n, int value) {
  set_int_word(n, value);
}

