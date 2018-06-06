/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file configPageManager.I
 * @author drose
 * @date 2004-10-15
 */

/**
 * Returns true if the implicit *.prc files have already been loaded, false
 * otherwise.  Normally this will only be false briefly before startup.
 */
INLINE bool ConfigPageManager::
loaded_implicit_pages() const {
  return _loaded_implicit;
}

/**
 * Searches the PRC_DIR and/or PRC_PATH directories for *.prc files and loads
 * them in as pages.  This is normally called automatically at startup time,
 * when the first variable's value is referenced.  See also
 * reload_implicit_pages().
 */
INLINE void ConfigPageManager::
load_implicit_pages() {
  if (!_loaded_implicit) {
    reload_implicit_pages();
  }
}

/**
 * Returns the search path used to locate implicit .prc files.  This is
 * determined by the PRC_DIR and PRC_PATH environment variables.  The object
 * returned by this method may be modified to change the path at runtime, and
 * then reload_implicit_pages() called.
 */
INLINE DSearchPath &ConfigPageManager::
get_search_path() {
  load_implicit_pages();
  return _search_path;
}

/**
 * Returns the number of patterns, like "*.prc", that are compiled in that
 * will be searched for as default config filenames.  Normally there is only
 * one pattern, and it is "*.prc", but others may be specified with the
 * PRC_FILENAME variable in Config.pp.
 */
INLINE size_t ConfigPageManager::
get_num_prc_patterns() const {
  return _prc_patterns.size();
}

/**
 * Returns the nth filename pattern that will be considered a match as a valid
 * config file.  See get_num_prc_patterns().
 */
INLINE std::string ConfigPageManager::
get_prc_pattern(size_t n) const {
  nassertr(n < _prc_patterns.size(), std::string());
  return _prc_patterns[n].get_pattern();
}

/**
 * Returns the number of patterns, like "*.pre", that are compiled in that
 * will be searched for as special config files that are understood to be
 * encrypted.
 */
INLINE size_t ConfigPageManager::
get_num_prc_encrypted_patterns() const {
  return _prc_encrypted_patterns.size();
}

/**
 * Returns the nth filename pattern that will be considered a match as a valid
 * encrypted config file.  See get_num_prc_encrypted_patterns().
 */
INLINE std::string ConfigPageManager::
get_prc_encrypted_pattern(size_t n) const {
  nassertr(n < _prc_patterns.size(), std::string());
  return _prc_encrypted_patterns[n].get_pattern();
}

/**
 * Returns the number of patterns, like "*.exe", that are compiled in that
 * will be searched for as special config files that are to be executed as a
 * program, and their output taken to be input.  This is normally empty.
 */
INLINE size_t ConfigPageManager::
get_num_prc_executable_patterns() const {
  return _prc_executable_patterns.size();
}

/**
 * Returns the nth filename pattern that will be considered a match as a valid
 * executable-style config file.  See get_num_prc_executable_patterns().
 */
INLINE std::string ConfigPageManager::
get_prc_executable_pattern(size_t n) const {
  nassertr(n < _prc_patterns.size(), std::string());
  return _prc_executable_patterns[n].get_pattern();
}

/**
 * Returns the current number of implicitly-loaded ConfigPages in the world.
 * These represent files that were automatically discovered on the disk as
 * .prc files.
 */
INLINE size_t ConfigPageManager::
get_num_implicit_pages() const {
  return _implicit_pages.size();
}

/**
 * Returns the nth implicit ConfigPage in the world.  See
 * get_num_implicit_pages().
 */
INLINE ConfigPage *ConfigPageManager::
get_implicit_page(size_t n) const {
  check_sort_pages();
  nassertr(n < _implicit_pages.size(), nullptr);
  return _implicit_pages[n];
}

/**
 * Returns the current number of explicitly-loaded ConfigPages in the world.
 * These represent pages that were loaded dynamically at runtime by explicit
 * calls to ConfigPageManager::make_explicit_page().
 */
INLINE size_t ConfigPageManager::
get_num_explicit_pages() const {
  return _explicit_pages.size();
}

/**
 * Returns the nth explicit ConfigPage in the world.  See
 * get_num_explicit_pages().
 */
INLINE ConfigPage *ConfigPageManager::
get_explicit_page(size_t n) const {
  check_sort_pages();
  nassertr(n < _explicit_pages.size(), nullptr);
  return _explicit_pages[n];
}


/**
 * This method is meant to be used internally to this module; there is no need
 * to call it directly.  It indicates that the sort values of some pages may
 * have changed and pages need to be re-sorted.
 */
INLINE void ConfigPageManager::
mark_unsorted() {
  _pages_sorted = false;
}

/**
 * Called internally to ensure that the list of pages is properly sorted.
 */
INLINE void ConfigPageManager::
check_sort_pages() const {
  if (!_pages_sorted) {
    ((ConfigPageManager *)this)->sort_pages();
  }
}

INLINE std::ostream &
operator << (std::ostream &out, const ConfigPageManager &pageMgr) {
  pageMgr.output(out);
  return out;
}
