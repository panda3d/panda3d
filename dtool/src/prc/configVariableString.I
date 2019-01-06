/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file configVariableString.I
 * @author drose
 * @date 2004-10-20
 */

/**
 *
 */
INLINE ConfigVariableString::
ConfigVariableString(const std::string &name) :
  ConfigVariable(name, VT_string),
  _local_modified(initial_invalid_cache())
{
  _core->set_used();
}

/**
 *
 */
INLINE ConfigVariableString::
ConfigVariableString(const std::string &name, const std::string &default_value,
                     const std::string &description, int flags) :
#ifdef PRC_SAVE_DESCRIPTIONS
  ConfigVariable(name, VT_string, description, flags),
#else
  ConfigVariable(name, VT_string, std::string(), flags),
#endif
  _local_modified(initial_invalid_cache())
{
  _core->set_default_value(default_value);
  _core->set_used();
}

/**
 * Reassigns the variable's local value.
 */
INLINE void ConfigVariableString::
operator = (const std::string &value) {
  set_value(value);
}

/**
 * Returns the variable's value.
 */
INLINE ConfigVariableString::
operator const std::string & () const {
  return get_value();
}

/**
 *
 */
INLINE const char *ConfigVariableString::
c_str() const {
  return get_value().c_str();
}

/**
 *
 */
INLINE bool ConfigVariableString::
empty() const {
  return get_value().empty();
}

/**
 *
 */
INLINE size_t ConfigVariableString::
length() const {
  return get_value().length();
}

/**
 *
 */
INLINE char ConfigVariableString::
operator [] (size_t n) const {
  assert(n < length());
  return get_value()[n];
}

/**
 *
 */
INLINE bool ConfigVariableString::
operator == (const std::string &other) const {
  return get_value() == other;
}

/**
 *
 */
INLINE bool ConfigVariableString::
operator != (const std::string &other) const {
  return get_value() != other;
}

/**
 *
 */
INLINE bool ConfigVariableString::
operator < (const std::string &other) const {
  return get_value() < other;
}

/**
 * Reassigns the variable's local value.
 */
INLINE void ConfigVariableString::
set_value(const std::string &value) {
  set_string_value(value);
}

/**
 * Returns the variable's value.
 */
INLINE const std::string &ConfigVariableString::
get_value() const {
  TAU_PROFILE("const string &ConfigVariableString::get_value() const", " ", TAU_USER);
  if (!is_cache_valid(_local_modified)) {
    ((ConfigVariableString *)this)->reload_cache();
  }
  return _cache;
}

/**
 * Returns the variable's default value.
 */
INLINE std::string ConfigVariableString::
get_default_value() const {
  const ConfigDeclaration *decl = ConfigVariable::get_default_value();
  if (decl != nullptr) {
    return decl->get_string_value();
  }
  return std::string();
}

/**
 * Returns the variable's nth value.
 */
INLINE std::string ConfigVariableString::
get_word(size_t n) const {
  return get_string_word(n);
}

/**
 * Reassigns the variable's nth value.  This makes a local copy of the
 * variable's overall value.
 */
INLINE void ConfigVariableString::
set_word(size_t n, const std::string &value) {
  set_string_word(n, value);
}
