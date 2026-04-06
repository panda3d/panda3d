/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file configVariable.I
 * @author drose
 * @date 2004-10-18
 */

/**
 * This constructor is only intended to be called from a specialized
 * ConfigVariableFoo derived class.
 */
INLINE ConfigVariable::
ConfigVariable(const std::string &name, ConfigVariable::ValueType value_type) :
  ConfigVariableBase(name, value_type)
{
}

/**
 * This constructor is only intended to be called from a specialized
 * ConfigVariableFoo derived class.
 */
INLINE ConfigVariable::
ConfigVariable(const std::string &name, ConfigVariable::ValueType value_type,
               const std::string &description, int flags) :
  ConfigVariableBase(name, value_type, description, flags)
{
}

/**
 * Use this constructor to make a ConfigVariable of an unspecified type.
 * Usually you'd want to do this just to reference a previously-defined
 * ConfigVariable of a specific type, without having to know what type it is.
 */
INLINE ConfigVariable::
ConfigVariable(const std::string &name) :
  ConfigVariableBase(name, VT_undefined)
{
  _core->set_used();
}

/**
 *
 */
INLINE ConfigVariable::
~ConfigVariable() {
}

/**
 * Returns the default variable specified for this variable.  If the variable
 * has not yet been defined, this will return NULL.
 */
INLINE const ConfigDeclaration *ConfigVariable::
get_default_value() const {
  nassertr(is_constructed(), nullptr);
  return _core->get_default_value();
}

/**
 * Returns the toplevel value of the variable, formatted as a string.
 */
INLINE const std::string &ConfigVariable::
get_string_value() const {
  nassertr(is_constructed(), *new std::string());
  const ConfigDeclaration *decl = _core->get_declaration(0);
  return decl->get_string_value();
}

/**
 * Changes the value assigned to this variable.  This creates a local value
 * that shadows any values defined in the .prc files, until
 * clear_local_value() is called.
 */
INLINE void ConfigVariable::
set_string_value(const std::string &string_value) {
  nassertv(is_constructed());
  _core->make_local_value()->set_string_value(string_value);
}

/**
 * Removes the value assigned to this variable, and lets its original value
 * (as read from the prc files) show through.
 */
INLINE void ConfigVariable::
clear_value() {
  nassertv(is_constructed());
  _core->clear_local_value();
}

/**
 * Returns the number of words in the variable's value.  A word is defined as
 * a sequence of non-whitespace characters delimited by whitespace.
 */
INLINE size_t ConfigVariable::
get_num_words() const {
  nassertr(is_constructed(), 0);
  const ConfigDeclaration *decl = _core->get_declaration(0);
  return decl->get_num_words();
}

/**
 * Returns true if the variable's value has a valid string value for the nth
 * word.  This is really the same thing as asking if there are at least n
 * words in the value.
 */
INLINE bool ConfigVariable::
has_string_word(size_t n) const {
  nassertr(is_constructed(), false);
  const ConfigDeclaration *decl = _core->get_declaration(0);
  return decl->has_string_word(n);
}

/**
 * Returns true if the variable's value has a valid boolean value for the nth
 * word.
 */
INLINE bool ConfigVariable::
has_bool_word(size_t n) const {
  nassertr(is_constructed(), false);
  const ConfigDeclaration *decl = _core->get_declaration(0);
  return decl->has_bool_word(n);
}

/**
 * Returns true if the variable's value has a valid integer value for the nth
 * word.
 */
INLINE bool ConfigVariable::
has_int_word(size_t n) const {
  nassertr(is_constructed(), false);
  const ConfigDeclaration *decl = _core->get_declaration(0);
  return decl->has_int_word(n);
}

/**
 * Returns true if the variable's value has a valid 64-bit integer value for
 * the nth word.
 */
INLINE bool ConfigVariable::
has_int64_word(size_t n) const {
  nassertr(is_constructed(), false);
  const ConfigDeclaration *decl = _core->get_declaration(0);
  return decl->has_int64_word(n);
}

/**
 * Returns true if the variable's value has a valid integer value for the nth
 * word.
 */
INLINE bool ConfigVariable::
has_double_word(size_t n) const {
  nassertr(is_constructed(), false);
  const ConfigDeclaration *decl = _core->get_declaration(0);
  return decl->has_double_word(n);
}

/**
 * Returns the string value of the nth word of the variable's value, or empty
 * string if there is no nth value.  See also has_string_word().
 */
INLINE std::string ConfigVariable::
get_string_word(size_t n) const {
  nassertr(is_constructed(), std::string());
  const ConfigDeclaration *decl = _core->get_declaration(0);
  return decl->get_string_word(n);
}

/**
 * Returns the boolean value of the nth word of the variable's value, or false
 * if there is no nth value.  See also has_bool_word().
 */
INLINE bool ConfigVariable::
get_bool_word(size_t n) const {
  nassertr(is_constructed(), false);
  const ConfigDeclaration *decl = _core->get_declaration(0);
  return decl->get_bool_word(n);
}

/**
 * Returns the integer value of the nth word of the variable's value, or 0 if
 * there is no nth value.  See also has_int_word().
 */
INLINE int ConfigVariable::
get_int_word(size_t n) const {
  nassertr(is_constructed(), 0);
  const ConfigDeclaration *decl = _core->get_declaration(0);
  return decl->get_int_word(n);
}

/**
 * Returns the int64 value of the nth word of the variable's value, or 0 if
 * there is no nth value.  See also has_int_word().
 */
INLINE int64_t ConfigVariable::
get_int64_word(size_t n) const {
  nassertr(is_constructed(), 0);
  const ConfigDeclaration *decl = _core->get_declaration(0);
  return decl->get_int64_word(n);
}

/**
 * Returns the integer value of the nth word of the variable's value, or 0 if
 * there is no nth value.  See also has_double_word().
 */
INLINE double ConfigVariable::
get_double_word(size_t n) const {
  nassertr(is_constructed(), 0.0);
  const ConfigDeclaration *decl = _core->get_declaration(0);
  return decl->get_double_word(n);
}

/**
 * Changes the nth word to the indicated value without affecting the other
 * words.
 */
INLINE void ConfigVariable::
set_string_word(size_t n, const std::string &value) {
  nassertv(is_constructed());
  _core->make_local_value()->set_string_word(n, value);
}

/**
 * Changes the nth word to the indicated value without affecting the other
 * words.
 */
INLINE void ConfigVariable::
set_bool_word(size_t n, bool value) {
  nassertv(is_constructed());
  _core->make_local_value()->set_bool_word(n, value);
}

/**
 * Changes the nth word to the indicated value without affecting the other
 * words.
 */
INLINE void ConfigVariable::
set_int_word(size_t n, int value) {
  nassertv(is_constructed());
  _core->make_local_value()->set_int_word(n, value);
}

/**
 * Changes the nth word to the indicated value without affecting the other
 * words.
 */
INLINE void ConfigVariable::
set_int64_word(size_t n, int64_t value) {
  nassertv(is_constructed());
  _core->make_local_value()->set_int64_word(n, value);
}

/**
 * Changes the nth word to the indicated value without affecting the other
 * words.
 */
INLINE void ConfigVariable::
set_double_word(size_t n, double value) {
  nassertv(is_constructed());
  _core->make_local_value()->set_double_word(n, value);
}

/**
 * Returns true if the constructor has been called and _core initialized,
 * false if the constructor has not yet been called and _core is NULL.  This
 * is intended to be placed in an assertion check, to guard against static-
 * init ordering issues.
 */
INLINE bool ConfigVariable::
is_constructed() const {
#ifndef NDEBUG
  if (_core == nullptr) {
    report_unconstructed();
    return false;
  }
#endif
  return true;
}
