/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file configVariableFilename.I
 * @author drose
 * @date 2004-11-22
 */

/**
 *
 */
INLINE ConfigVariableFilename::
ConfigVariableFilename(const std::string &name) :
  ConfigVariable(name, VT_filename),
  _local_modified(initial_invalid_cache())
{
  _core->set_used();
}

/**
 *
 */
INLINE ConfigVariableFilename::
ConfigVariableFilename(const std::string &name, const Filename &default_value,
                       const std::string &description, int flags) :
#ifdef PRC_SAVE_DESCRIPTIONS
  ConfigVariable(name, VT_filename, description, flags),
#else
  ConfigVariable(name, VT_filename, std::string(), flags),
#endif
  _local_modified(initial_invalid_cache())
{
  _core->set_default_value(default_value);
  _core->set_used();
}

/**
 * Reassigns the variable's local value.
 */
INLINE void ConfigVariableFilename::
operator = (const Filename &value) {
  set_value(value);
}

/**
 * Returns the variable's value as a Filename.
 */
INLINE ConfigVariableFilename::
operator const Filename &() const {
  return get_ref_value();
}

/**
 *
 */
INLINE const char *ConfigVariableFilename::
c_str() const {
  return get_ref_value().c_str();
}

/**
 *
 */
INLINE bool ConfigVariableFilename::
empty() const {
  return get_ref_value().empty();
}

/**
 *
 */
INLINE size_t ConfigVariableFilename::
length() const {
  return get_ref_value().length();
}

/**
 *
 */
INLINE char ConfigVariableFilename::
operator [] (size_t n) const {
  return get_ref_value()[n];
}


/**
 * Returns the entire filename: directory, basename, extension.  This is the
 * same thing returned by the string typecast operator, so this function is a
 * little redundant.
 */
INLINE std::string ConfigVariableFilename::
get_fullpath() const {
  return get_ref_value().get_fullpath();
}

/**
 * Returns the directory part of the filename.  This is everything in the
 * filename up to, but not including the rightmost slash.
 */
INLINE std::string ConfigVariableFilename::
get_dirname() const {
  return get_ref_value().get_dirname();
}

/**
 * Returns the basename part of the filename.  This is everything in the
 * filename after the rightmost slash, including any extensions.
 */
INLINE std::string ConfigVariableFilename::
get_basename() const {
  return get_ref_value().get_basename();
}


/**
 * Returns the full filename--directory and basename parts--except for the
 * extension.
 */
INLINE std::string ConfigVariableFilename::
get_fullpath_wo_extension() const {
  return get_ref_value().get_fullpath_wo_extension();
}


/**
 * Returns the basename part of the filename, without the file extension.
 */
INLINE std::string ConfigVariableFilename::
get_basename_wo_extension() const {
  return get_ref_value().get_basename_wo_extension();
}


/**
 * Returns the file extension.  This is everything after the rightmost dot, if
 * there is one, or the empty string if there is not.
 */
INLINE std::string ConfigVariableFilename::
get_extension() const {
  return get_ref_value().get_extension();
}

/**
 *
 */
INLINE bool ConfigVariableFilename::
operator == (const Filename &other) const {
  return get_ref_value() == other;
}

/**
 *
 */
INLINE bool ConfigVariableFilename::
operator != (const Filename &other) const {
  return get_ref_value() != other;
}

/**
 *
 */
INLINE bool ConfigVariableFilename::
operator < (const Filename &other) const {
  return get_ref_value() < other;
}

/**
 * Reassigns the variable's local value.
 */
INLINE void ConfigVariableFilename::
set_value(const Filename &value) {
  set_string_value(value);
}

/**
 * Returns the variable's value.
 */
INLINE Filename ConfigVariableFilename::
get_value() const {
  // This returns a concrete rather than a reference by design, to avoid
  // problems with scope.  When we call this method from Python, we'd like to
  // be able to keep the Filename value around longer than the lifetime of the
  // config variable itself.
  return get_ref_value();
}

/**
 * Returns the variable's default value.
 */
INLINE Filename ConfigVariableFilename::
get_default_value() const {
  const ConfigDeclaration *decl = ConfigVariable::get_default_value();
  if (decl != nullptr) {
    return Filename::expand_from(decl->get_string_value());
  }
  return Filename();
}

/**
 * Returns the variable's nth value.
 */
INLINE Filename ConfigVariableFilename::
get_word(size_t n) const {
  return Filename::expand_from(get_string_word(n));
}

/**
 * Reassigns the variable's nth value.  This makes a local copy of the
 * variable's overall value.
 */
INLINE void ConfigVariableFilename::
set_word(size_t n, const Filename &value) {
  set_string_word(n, value);
}

/**
 * Returns the variable's value, as a reference into the config variable
 * itself.  This is the internal method that implements get_value(), which
 * returns a concrete.
 */
INLINE const Filename &ConfigVariableFilename::
get_ref_value() const {
  TAU_PROFILE("const Filename &ConfigVariableFilename::get_ref_value() const", " ", TAU_USER);
  if (!is_cache_valid(_local_modified)) {
    ((ConfigVariableFilename *)this)->reload_cache();
  }
  return _cache;
}
