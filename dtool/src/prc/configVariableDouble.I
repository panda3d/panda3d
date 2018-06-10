/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file configVariableDouble.I
 * @author drose
 * @date 2004-10-20
 */

/**
 *
 */
INLINE ConfigVariableDouble::
ConfigVariableDouble(const std::string &name) :
  ConfigVariable(name, VT_double),
  _local_modified(initial_invalid_cache())
{
  _core->set_used();
}

/**
 *
 */
INLINE ConfigVariableDouble::
ConfigVariableDouble(const std::string &name, double default_value,
                     const std::string &description, int flags) :
#ifdef PRC_SAVE_DESCRIPTIONS
  ConfigVariable(name, ConfigVariableCore::VT_double, description, flags),
#else
  ConfigVariable(name, ConfigVariableCore::VT_double, std::string(), flags),
#endif
  _local_modified(initial_invalid_cache())
{
  set_default_value(default_value);
  _core->set_used();
}

/**
 *
 */
INLINE ConfigVariableDouble::
ConfigVariableDouble(const std::string &name, const std::string &default_value,
                     const std::string &description, int flags) :
#ifdef PRC_SAVE_DESCRIPTIONS
  ConfigVariable(name, ConfigVariableCore::VT_double, description, flags),
#else
  ConfigVariable(name, ConfigVariableCore::VT_double, std::string(), flags),
#endif
  _local_modified(initial_invalid_cache())
{
  _core->set_default_value(default_value);
  _core->set_used();
}

/**
 * Reassigns the variable's local value.
 */
INLINE void ConfigVariableDouble::
operator = (double value) {
  set_value(value);
}

/**
 * Returns the variable's value.
 */
INLINE ConfigVariableDouble::
operator double () const {
  return get_value();
}

/**
 * Returns the number of unique words in the variable.
 */
INLINE size_t ConfigVariableDouble::
size() const {
  return get_num_words();
}

/**
 * Returns the value of the variable's nth word.
 */
INLINE double ConfigVariableDouble::
operator [] (size_t n) const {
  return get_word(n);
}

/**
 * Reassigns the variable's local value.
 */
INLINE void ConfigVariableDouble::
set_value(double value) {
  set_string_value("");
  set_double_word(0, value);
}

/**
 * Returns the variable's value.
 */
INLINE double ConfigVariableDouble::
get_value() const {
  TAU_PROFILE("double ConfigVariableDouble::get_value() const", " ", TAU_USER);
  if (!is_cache_valid(_local_modified)) {
    mark_cache_valid(((ConfigVariableDouble *)this)->_local_modified);
    ((ConfigVariableDouble *)this)->_cache = get_double_word(0);
  }
  return _cache;
}

/**
 * Returns the variable's default value.
 */
INLINE double ConfigVariableDouble::
get_default_value() const {
  const ConfigDeclaration *decl = ConfigVariable::get_default_value();
  if (decl != nullptr) {
    return decl->get_double_word(0);
  }
  return 0.0;
}

/**
 * Returns the variable's nth value.
 */
INLINE double ConfigVariableDouble::
get_word(size_t n) const {
  return get_double_word(n);
}

/**
 * Reassigns the variable's nth value.  This makes a local copy of the
 * variable's overall value.
 */
INLINE void ConfigVariableDouble::
set_word(size_t n, double value) {
  set_double_word(n, value);
}
