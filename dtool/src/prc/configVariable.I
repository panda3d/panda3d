// Filename: configVariable.I
// Created by:  drose (18Oct04)
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
//     Function: ConfigVariable::Constructor
//       Access: Protected
//  Description: This constructor is only intended to be called from a
//               specialized ConfigVariableFoo derived class.
////////////////////////////////////////////////////////////////////
INLINE ConfigVariable::
ConfigVariable(const string &name, ConfigVariable::ValueType value_type) :
  ConfigVariableBase(name, value_type)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariable::Constructor
//       Access: Protected
//  Description: This constructor is only intended to be called from a
//               specialized ConfigVariableFoo derived class.
////////////////////////////////////////////////////////////////////
INLINE ConfigVariable::
ConfigVariable(const string &name, ConfigVariable::ValueType value_type,
               const string &description, int flags) :
  ConfigVariableBase(name, value_type, description, flags)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariable::Constructor
//       Access: Published
//  Description: Use this constructor to make a ConfigVariable of an
//               unspecified type.  Usually you'd want to do this just
//               to reference a previously-defined ConfigVariable of a
//               specific type, without having to know what type it is.
////////////////////////////////////////////////////////////////////
INLINE ConfigVariable::
ConfigVariable(const string &name) :
  ConfigVariableBase(name, VT_undefined)
{
  _core->set_used();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariable::Destructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ConfigVariable::
~ConfigVariable() {
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariable::get_default_value
//       Access: Published
//  Description: Returns the default variable specified for this
//               variable.  If the variable has not yet been defined,
//               this will return NULL.
////////////////////////////////////////////////////////////////////
INLINE const ConfigDeclaration *ConfigVariable::
get_default_value() const {
  nassertr(is_constructed(), (ConfigDeclaration *)NULL);
  return _core->get_default_value();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariable::get_string_value
//       Access: Published
//  Description: Returns the toplevel value of the variable, formatted
//               as a string.
////////////////////////////////////////////////////////////////////
INLINE const string &ConfigVariable::
get_string_value() const {
  nassertr(is_constructed(), *new string());
  const ConfigDeclaration *decl = _core->get_declaration(0);
  return decl->get_string_value();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariable::set_string_value
//       Access: Published
//  Description: Changes the value assigned to this variable.  This
//               creates a local value that shadows any values defined
//               in the .prc files, until clear_local_value() is
//               called.
////////////////////////////////////////////////////////////////////
INLINE void ConfigVariable::
set_string_value(const string &string_value) {
  nassertv(is_constructed());
  _core->make_local_value()->set_string_value(string_value);
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariable::clear_value
//       Access: Published
//  Description: Removes the value assigned to this variable, and lets
//               its original value (as read from the prc files) show
//               through.
////////////////////////////////////////////////////////////////////
INLINE void ConfigVariable::
clear_value() {
  nassertv(is_constructed());
  _core->clear_local_value();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariable::get_num_words
//       Access: Published
//  Description: Returns the number of words in the variable's
//               value.  A word is defined as a sequence of
//               non-whitespace characters delimited by whitespace.
////////////////////////////////////////////////////////////////////
INLINE int ConfigVariable::
get_num_words() const {
  nassertr(is_constructed(), 0);
  const ConfigDeclaration *decl = _core->get_declaration(0);
  return decl->get_num_words();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariable::has_string_word
//       Access: Published
//  Description: Returns true if the variable's value has a valid
//               string value for the nth word.  This is really the
//               same thing as asking if there are at least n words in
//               the value.
////////////////////////////////////////////////////////////////////
INLINE bool ConfigVariable::
has_string_word(int n) const {
  nassertr(is_constructed(), false);
  const ConfigDeclaration *decl = _core->get_declaration(0);
  return decl->has_string_word(n);
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariable::has_bool_word
//       Access: Published
//  Description: Returns true if the variable's value has a valid
//               boolean value for the nth word.
////////////////////////////////////////////////////////////////////
INLINE bool ConfigVariable::
has_bool_word(int n) const {
  nassertr(is_constructed(), false);
  const ConfigDeclaration *decl = _core->get_declaration(0);
  return decl->has_bool_word(n);
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariable::has_int_word
//       Access: Published
//  Description: Returns true if the variable's value has a valid
//               integer value for the nth word.
////////////////////////////////////////////////////////////////////
INLINE bool ConfigVariable::
has_int_word(int n) const {
  nassertr(is_constructed(), false);
  const ConfigDeclaration *decl = _core->get_declaration(0);
  return decl->has_int_word(n);
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariable::has_int64_word
//       Access: Published
//  Description: Returns true if the variable's value has a valid
//               64-bit integer value for the nth word.
////////////////////////////////////////////////////////////////////
INLINE bool ConfigVariable::
has_int64_word(int n) const {
  nassertr(is_constructed(), false);
  const ConfigDeclaration *decl = _core->get_declaration(0);
  return decl->has_int64_word(n);
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariable::has_double_word
//       Access: Published
//  Description: Returns true if the variable's value has a valid
//               integer value for the nth word.
////////////////////////////////////////////////////////////////////
INLINE bool ConfigVariable::
has_double_word(int n) const {
  nassertr(is_constructed(), false);
  const ConfigDeclaration *decl = _core->get_declaration(0);
  return decl->has_double_word(n);
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariable::get_string_word
//       Access: Published
//  Description: Returns the string value of the nth word of the
//               variable's value, or empty string if there is no
//               nth value.  See also has_string_word().
////////////////////////////////////////////////////////////////////
INLINE string ConfigVariable::
get_string_word(int n) const {
  nassertr(is_constructed(), string());
  const ConfigDeclaration *decl = _core->get_declaration(0);
  return decl->get_string_word(n);
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariable::get_bool_word
//       Access: Published
//  Description: Returns the boolean value of the nth word of the
//               variable's value, or false if there is no nth
//               value.  See also has_bool_word().
////////////////////////////////////////////////////////////////////
INLINE bool ConfigVariable::
get_bool_word(int n) const {
  nassertr(is_constructed(), false);
  const ConfigDeclaration *decl = _core->get_declaration(0);
  return decl->get_bool_word(n);
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariable::get_int_word
//       Access: Published
//  Description: Returns the integer value of the nth word of the
//               variable's value, or 0 if there is no nth value.
//               See also has_int_word().
////////////////////////////////////////////////////////////////////
INLINE int ConfigVariable::
get_int_word(int n) const {
  nassertr(is_constructed(), 0);
  const ConfigDeclaration *decl = _core->get_declaration(0);
  return decl->get_int_word(n);
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariable::get_int64_word
//       Access: Published
//  Description: Returns the int64 value of the nth word of the
//               variable's value, or 0 if there is no nth value.
//               See also has_int_word().
////////////////////////////////////////////////////////////////////
INLINE PN_int64 ConfigVariable::
get_int64_word(int n) const {
  nassertr(is_constructed(), 0);
  const ConfigDeclaration *decl = _core->get_declaration(0);
  return decl->get_int64_word(n);
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariable::get_double_word
//       Access: Published
//  Description: Returns the integer value of the nth word of the
//               variable's value, or 0 if there is no nth value.
//               See also has_double_word().
////////////////////////////////////////////////////////////////////
INLINE double ConfigVariable::
get_double_word(int n) const {
  nassertr(is_constructed(), 0.0);
  const ConfigDeclaration *decl = _core->get_declaration(0);
  return decl->get_double_word(n);
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariable::set_string_word
//       Access: Published
//  Description: Changes the nth word to the indicated value without
//               affecting the other words.
////////////////////////////////////////////////////////////////////
INLINE void ConfigVariable::
set_string_word(int n, const string &value) {
  nassertv(is_constructed());
  _core->make_local_value()->set_string_word(n, value);
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariable::set_bool_word
//       Access: Published
//  Description: Changes the nth word to the indicated value without
//               affecting the other words.
////////////////////////////////////////////////////////////////////
INLINE void ConfigVariable::
set_bool_word(int n, bool value) {
  nassertv(is_constructed());
  _core->make_local_value()->set_bool_word(n, value);
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariable::set_int_word
//       Access: Published
//  Description: Changes the nth word to the indicated value without
//               affecting the other words.
////////////////////////////////////////////////////////////////////
INLINE void ConfigVariable::
set_int_word(int n, int value) {
  nassertv(is_constructed());
  _core->make_local_value()->set_int_word(n, value);
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariable::set_int64_word
//       Access: Published
//  Description: Changes the nth word to the indicated value without
//               affecting the other words.
////////////////////////////////////////////////////////////////////
INLINE void ConfigVariable::
set_int64_word(int n, PN_int64 value) {
  nassertv(is_constructed());
  _core->make_local_value()->set_int_word(n, value);
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariable::set_double_word
//       Access: Published
//  Description: Changes the nth word to the indicated value without
//               affecting the other words.
////////////////////////////////////////////////////////////////////
INLINE void ConfigVariable::
set_double_word(int n, double value) {
  nassertv(is_constructed());
  _core->make_local_value()->set_double_word(n, value);
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariable::is_constructed
//       Access: Protected
//  Description: Returns true if the constructor has been called and
//               _core initialized, false if the constructor has not
//               yet been called and _core is NULL.  This is intended
//               to be placed in an assertion check, to guard against
//               static-init ordering issues.
////////////////////////////////////////////////////////////////////
INLINE bool ConfigVariable::
is_constructed() const {
#ifndef NDEBUG
  if (_core == (ConfigVariableCore *)NULL) {
    report_unconstructed();
    return false;
  }
#endif
  return true;
}
