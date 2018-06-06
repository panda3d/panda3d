/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file configVariableInt64.I
 * @author drose
 * @date 2007-12-19
 */

/**
 *
 */
INLINE ConfigVariableInt64::
ConfigVariableInt64(const std::string &name) :
  ConfigVariable(name, VT_int64),
  _local_modified(initial_invalid_cache())
{
  _core->set_used();
}

/**
 *
 */
INLINE ConfigVariableInt64::
ConfigVariableInt64(const std::string &name, int64_t default_value,
                    const std::string &description, int flags) :
#ifdef PRC_SAVE_DESCRIPTIONS
  ConfigVariable(name, ConfigVariableCore::VT_int64, description, flags),
#else
  ConfigVariable(name, ConfigVariableCore::VT_int64, std::string(), flags),
#endif
  _local_modified(initial_invalid_cache())
{
  set_default_value(default_value);
  _core->set_used();
}

/**
 *
 */
INLINE ConfigVariableInt64::
ConfigVariableInt64(const std::string &name, const std::string &default_value,
                    const std::string &description, int flags) :
#ifdef PRC_SAVE_DESCRIPTIONS
  ConfigVariable(name, ConfigVariableCore::VT_int64, description, flags),
#else
  ConfigVariable(name, ConfigVariableCore::VT_int64, std::string(), flags),
#endif
  _local_modified(initial_invalid_cache())
{
  _core->set_default_value(default_value);
  _core->set_used();
}

/**
 * Reassigns the variable's local value.
 */
INLINE void ConfigVariableInt64::
operator = (int64_t value) {
  set_value(value);
}

/**
 * Returns the variable's value.
 */
INLINE ConfigVariableInt64::
operator int64_t () const {
  return get_value();
}

/**
 * Returns the number of unique words in the variable.
 */
INLINE size_t ConfigVariableInt64::
size() const {
  return get_num_words();
}

/**
 * Returns the value of the variable's nth word.
 */
INLINE int64_t ConfigVariableInt64::
operator [] (size_t n) const {
  return get_word(n);
}

/**
 * Reassigns the variable's local value.
 */
INLINE void ConfigVariableInt64::
set_value(int64_t value) {
  set_string_value("");
  set_int64_word(0, value);
}

/**
 * Returns the variable's value.
 */
INLINE int64_t ConfigVariableInt64::
get_value() const {
  TAU_PROFILE("int64_t ConfigVariableInt64::get_value() const", " ", TAU_USER);
  if (!is_cache_valid(_local_modified)) {
    mark_cache_valid(((ConfigVariableInt64 *)this)->_local_modified);
    ((ConfigVariableInt64 *)this)->_cache = get_int64_word(0);
  }
  return _cache;
}

/**
 * Returns the variable's default value.
 */
INLINE int64_t ConfigVariableInt64::
get_default_value() const {
  const ConfigDeclaration *decl = ConfigVariable::get_default_value();
  if (decl != nullptr) {
    return decl->get_int64_word(0);
  }
  return 0;
}

/**
 * Returns the variable's nth value.
 */
INLINE int64_t ConfigVariableInt64::
get_word(size_t n) const {
  return get_int64_word(n);
}

/**
 * Reassigns the variable's nth value.  This makes a local copy of the
 * variable's overall value.
 */
INLINE void ConfigVariableInt64::
set_word(size_t n, int64_t value) {
  set_int64_word(n, value);
}
