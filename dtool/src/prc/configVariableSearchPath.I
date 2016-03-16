/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file configVariableSearchPath.I
 * @author drose
 * @date 2004-10-21
 */

/**
 *
 */
INLINE ConfigVariableSearchPath::
ConfigVariableSearchPath(const string &name,
                         const string &description, int flags) :
#ifdef PRC_SAVE_DESCRIPTIONS
  ConfigVariableBase(name, VT_search_path, description, flags),
#else
  ConfigVariableBase(name, VT_search_path, string(), flags),
#endif
  _default_value(Filename(".")),
  _local_modified(initial_invalid_cache())
{
  // A SearchPath variable implicitly defines a default value of the empty
  // string.  This is just to prevent the core variable from complaining
  // should anyone ask for its solitary value.
  if (_core->get_default_value() == (ConfigDeclaration *)NULL) {
    _core->set_default_value("");
  }
  _core->set_used();
}

/**
 *
 */
INLINE ConfigVariableSearchPath::
ConfigVariableSearchPath(const string &name,
                         const DSearchPath &default_value,
                         const string &description, int flags) :
#ifdef PRC_SAVE_DESCRIPTIONS
  ConfigVariableBase(name, VT_search_path, description, flags),
#else
  ConfigVariableBase(name, VT_search_path, string(), flags),
#endif
  _default_value(default_value),
  _local_modified(initial_invalid_cache())
{
  // A SearchPath variable implicitly defines a default value of the empty
  // string.  This is just to prevent the core variable from complaining
  // should anyone ask for its solitary value.
  if (_core->get_default_value() == (ConfigDeclaration *)NULL) {
    _core->set_default_value("");
  }
  _core->set_used();
}

/**
 *
 */
INLINE ConfigVariableSearchPath::
ConfigVariableSearchPath(const string &name,
                         const string &default_value,
                         const string &description, int flags) :
#ifdef PRC_SAVE_DESCRIPTIONS
  ConfigVariableBase(name, VT_search_path, description, flags),
#else
  ConfigVariableBase(name, VT_search_path, string(), flags),
#endif
  _default_value(Filename(default_value)),
  _local_modified(initial_invalid_cache())
{
  // A SearchPath variable implicitly defines a default value of the empty
  // string.  This is just to prevent the core variable from complaining
  // should anyone ask for its solitary value.
  if (_core->get_default_value() == (ConfigDeclaration *)NULL) {
    _core->set_default_value("");
  }
  _core->set_used();
}

/**
 *
 */
INLINE ConfigVariableSearchPath::
~ConfigVariableSearchPath() {
}

/**
 * Returns the variable's value.
 */
INLINE ConfigVariableSearchPath::
operator const DSearchPath & () const {
  return get_value();
}

/**
 *
 */
INLINE const DSearchPath &ConfigVariableSearchPath::
get_value() const {
  TAU_PROFILE("const DSearchPath &ConfigVariableSearchPath::get_value() const", " ", TAU_USER);
  if (!is_cache_valid(_local_modified)) {
    ((ConfigVariableSearchPath *)this)->reload_search_path();
  }
  return _cache;
}

/**
 *
 */
INLINE const DSearchPath &ConfigVariableSearchPath::
get_default_value() const {
  return _default_value;
}

/**
 * Removes all the directories locally added to the search list, and restores
 * it to its original form.
 */
INLINE bool ConfigVariableSearchPath::
clear_local_value() {
  nassertr(_core != (ConfigVariableCore *)NULL, false);

  bool any_to_clear = !_prefix.is_empty() || _postfix.is_empty();
  _prefix.clear();
  _postfix.clear();

  if (_core->clear_local_value()) {
    any_to_clear = true;
  }

  _local_modified = initial_invalid_cache();
  return any_to_clear;
}

/**
 * Removes all the directories locally added to the search list, and restores
 * it to its original form.
 */
INLINE void ConfigVariableSearchPath::
clear() {
  clear_local_value();
}

/**
 * Adds a new directory to the end of the search list.
 */
INLINE void ConfigVariableSearchPath::
append_directory(const Filename &directory) {
  _postfix.append_directory(directory);
  _local_modified = initial_invalid_cache();
}

/**
 * Adds a new directory to the front of the search list.
 */
INLINE void ConfigVariableSearchPath::
prepend_directory(const Filename &directory) {
  _prefix.prepend_directory(directory);
  _local_modified = initial_invalid_cache();
}

/**
 * Adds all of the directories listed in the search path to the end of the
 * search list.
 */
INLINE void ConfigVariableSearchPath::
append_path(const string &path, const string &separator) {
  _postfix.append_path(path, separator);
  _local_modified = initial_invalid_cache();
}

/**
 * Adds all of the directories listed in the search path to the end of the
 * search list.
 */
INLINE void ConfigVariableSearchPath::
append_path(const DSearchPath &path) {
  _postfix.append_path(path);
  _local_modified = initial_invalid_cache();
}

/**
 * Adds all of the directories listed in the search path to the beginning of
 * the search list.
 */
INLINE void ConfigVariableSearchPath::
prepend_path(const DSearchPath &path) {
  _prefix.prepend_path(path);
  _local_modified = initial_invalid_cache();
}

/**
 * Returns true if the search list is empty, false otherwise.
 */
INLINE bool ConfigVariableSearchPath::
is_empty() const {
  return get_value().is_empty();
}

/**
 * Returns the number of directories on the search list.
 */
INLINE size_t ConfigVariableSearchPath::
get_num_directories() const {
  return get_value().get_num_directories();
}

/**
 * Returns the nth directory on the search list.
 */
INLINE const Filename &ConfigVariableSearchPath::
get_directory(size_t n) const {
  return get_value().get_directory(n);
}

/**
 * Searches all the directories in the search list for the indicated file, in
 * order.  Returns the full matching pathname of the first match if found, or
 * the empty string if not found.
 */
INLINE Filename ConfigVariableSearchPath::
find_file(const Filename &filename) const {
  return get_value().find_file(filename);
}

/**
 * Searches all the directories in the search list for the indicated file, in
 * order.  Fills up the results list with *all* of the matching filenames
 * found, if any.  Returns the number of matches found.
 *
 * It is the responsibility of the the caller to clear the results list first;
 * otherwise, the newly-found files will be appended to the list.
 */
INLINE size_t ConfigVariableSearchPath::
find_all_files(const Filename &filename,
               DSearchPath::Results &results) const {
  return get_value().find_all_files(filename, results);
}

/**
 * This variant of find_all_files() returns the new Results object, instead of
 * filling on in on the parameter list.  This is a little more convenient to
 * call from Python.
 */
INLINE DSearchPath::Results ConfigVariableSearchPath::
find_all_files(const Filename &filename) const {
  return get_value().find_all_files(filename);
}

/**
 *
 */
INLINE void ConfigVariableSearchPath::
output(ostream &out) const {
  get_value().output(out);
}

/**
 *
 */
INLINE void ConfigVariableSearchPath::
write(ostream &out) const {
  get_value().write(out);
}

INLINE ostream &
operator << (ostream &out, const ConfigVariableSearchPath &variable) {
  variable.output(out);
  return out;
}
