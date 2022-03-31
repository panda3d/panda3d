/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file configVariableBool.I
 * @author drose
 * @date 2004-10-20
 */

/**
 *
 */
INLINE ConfigVariableBool::
ConfigVariableBool(const std::string &name) :
  ConfigVariable(name, VT_bool),
  _local_modified(initial_invalid_cache())
{
  _core->set_used();
}

/**
 *
 */
INLINE ConfigVariableBool::
ConfigVariableBool(const std::string &name, bool default_value,
                   const std::string &description, int flags) :
#ifdef PRC_SAVE_DESCRIPTIONS
  ConfigVariable(name, VT_bool, description, flags),
#else
  ConfigVariable(name, VT_bool, std::string(), flags),
#endif
  _local_modified(initial_invalid_cache())
{
  _core->set_default_value(default_value ? "1" : "0");
  _core->set_used();
}

/**
 *
 */
INLINE ConfigVariableBool::
ConfigVariableBool(const std::string &name, const std::string &default_value,
                   const std::string &description, int flags) :
#ifdef PRC_SAVE_DESCRIPTIONS
  ConfigVariable(name, VT_bool, description, flags),
#else
  ConfigVariable(name, VT_bool, std::string(), flags),
#endif
  _local_modified(initial_invalid_cache())
{
  _core->set_default_value(default_value);
  _core->set_used();
}

/**
 * Reassigns the variable's local value.
 */
INLINE void ConfigVariableBool::
operator = (bool value) {
  set_value(value);
}

/**
 * Returns the variable's value.
 */
ALWAYS_INLINE ConfigVariableBool::
operator bool () const {
  return get_value();
}

/**
 * Returns the number of unique words in the variable.
 */
INLINE size_t ConfigVariableBool::
size() const {
  return get_num_words();
}

/**
 * Returns the value of the variable's nth word.
 */
INLINE bool ConfigVariableBool::
operator [] (size_t n) const {
  return get_word(n);
}

/**
 * Reassigns the variable's local value.
 */
INLINE void ConfigVariableBool::
set_value(bool value) {
  set_string_value("");
  set_bool_word(0, value);
}

/**
 * Returns the variable's value.
 */
ALWAYS_INLINE bool ConfigVariableBool::
get_value() const {
  TAU_PROFILE("bool ConfigVariableBool::get_value() const", " ", TAU_USER);
  if (!is_cache_valid(_local_modified)) {
    reload_value();
  }
  return _cache;
}

/**
 * Returns the variable's default value.
 */
INLINE bool ConfigVariableBool::
get_default_value() const {
  const ConfigDeclaration *decl = ConfigVariable::get_default_value();
  if (decl != nullptr) {
    return decl->get_bool_word(0);
  }
  return false;
}

/**
 * Returns the variable's nth value.
 */
INLINE bool ConfigVariableBool::
get_word(size_t n) const {
  return get_bool_word(n);
}

/**
 * Reassigns the variable's nth value.  This makes a local copy of the
 * variable's overall value.
 */
INLINE void ConfigVariableBool::
set_word(size_t n, bool value) {
  set_bool_word(n, value);
}
