// Filename: configPageManager.I
// Created by:  drose (15Oct04)
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
//     Function: ConfigPageManager::loaded_implicit_pages
//       Access: Published
//  Description: Returns true if the implicit *.prc files have already
//               been loaded, false otherwise.  Normally this will
//               only be false briefly before startup.
////////////////////////////////////////////////////////////////////
INLINE bool ConfigPageManager::
loaded_implicit_pages() const {
  return _loaded_implicit;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigPageManager::load_implicit_pages
//       Access: Published
//  Description: Searches the PRC_DIR and/or PRC_PATH directories for
//               *.prc files and loads them in as pages.  This is
//               normally called automatically at startup time, when
//               the first variable's value is referenced.  See also
//               reload_implicit_pages().
////////////////////////////////////////////////////////////////////
INLINE void ConfigPageManager::
load_implicit_pages() {
  if (!_loaded_implicit) {
    reload_implicit_pages();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigPageManager::get_search_path
//       Access: Published
//  Description: Returns the search path used to locate implicit .prc
//               files.  This is determined by the PRC_DIR and
//               PRC_PATH environment variables.  The object returned
//               by this method may be modified to change the path at
//               runtime, and then reload_implicit_pages() called.
////////////////////////////////////////////////////////////////////
INLINE DSearchPath &ConfigPageManager::
get_search_path() {
  load_implicit_pages();
  return _search_path;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigPageManager::get_num_prc_patterns
//       Access: Published
//  Description: Returns the number of patterns, like "*.prc", that
//               are compiled in that will be searched for as default
//               config filenames.  Normally there is only one
//               pattern, and it is "*.prc", but others may be
//               specified with the PRC_FILENAME variable in
//               Config.pp.
////////////////////////////////////////////////////////////////////
INLINE int ConfigPageManager::
get_num_prc_patterns() const {
  return _prc_patterns.size();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigPageManager::get_prc_pattern
//       Access: Published
//  Description: Returns the nth filename pattern that will be
//               considered a match as a valid config file.  See
//               get_num_prc_patterns().
////////////////////////////////////////////////////////////////////
INLINE string ConfigPageManager::
get_prc_pattern(int n) const {
  nassertr(n >= 0 && n < (int)_prc_patterns.size(), string());
  return _prc_patterns[n].get_pattern();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigPageManager::get_num_prc_encrypted_patterns
//       Access: Published
//  Description: Returns the number of patterns, like "*.pre", that
//               are compiled in that will be searched for as special
//               config files that are understood to be encrypted.
////////////////////////////////////////////////////////////////////
INLINE int ConfigPageManager::
get_num_prc_encrypted_patterns() const {
  return _prc_encrypted_patterns.size();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigPageManager::get_prc_encrypted_pattern
//       Access: Published
//  Description: Returns the nth filename pattern that will be
//               considered a match as a valid encrypted config
//               file.  See get_num_prc_encrypted_patterns().
////////////////////////////////////////////////////////////////////
INLINE string ConfigPageManager::
get_prc_encrypted_pattern(int n) const {
  nassertr(n >= 0 && n < (int)_prc_patterns.size(), string());
  return _prc_encrypted_patterns[n].get_pattern();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigPageManager::get_num_prc_executable_patterns
//       Access: Published
//  Description: Returns the number of patterns, like "*.exe", that
//               are compiled in that will be searched for as special
//               config files that are to be executed as a program,
//               and their output taken to be input.  This is normally
//               empty.
////////////////////////////////////////////////////////////////////
INLINE int ConfigPageManager::
get_num_prc_executable_patterns() const {
  return _prc_executable_patterns.size();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigPageManager::get_prc_executable_pattern
//       Access: Published
//  Description: Returns the nth filename pattern that will be
//               considered a match as a valid executable-style config
//               file.  See get_num_prc_executable_patterns().
////////////////////////////////////////////////////////////////////
INLINE string ConfigPageManager::
get_prc_executable_pattern(int n) const {
  nassertr(n >= 0 && n < (int)_prc_patterns.size(), string());
  return _prc_executable_patterns[n].get_pattern();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigPageManager::get_num_implicit_pages
//       Access: Published
//  Description: Returns the current number of implicitly-loaded
//               ConfigPages in the world.  These represent files that
//               were automatically discovered on the disk as .prc
//               files.
////////////////////////////////////////////////////////////////////
INLINE int ConfigPageManager::
get_num_implicit_pages() const {
  return _implicit_pages.size();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigPageManager::get_implicit_page
//       Access: Published
//  Description: Returns the nth implicit ConfigPage in the world.
//               See get_num_implicit_pages().
////////////////////////////////////////////////////////////////////
INLINE ConfigPage *ConfigPageManager::
get_implicit_page(int n) const {
  check_sort_pages();
  nassertr(n >= 0 && n < (int)_implicit_pages.size(), (ConfigPage *)NULL);
  return _implicit_pages[n];
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigPageManager::get_num_explicit_pages
//       Access: Published
//  Description: Returns the current number of explicitly-loaded
//               ConfigPages in the world.  These represent pages that
//               were loaded dynamically at runtime by explicit calls
//               to ConfigPageManager::make_explicit_page().
////////////////////////////////////////////////////////////////////
INLINE int ConfigPageManager::
get_num_explicit_pages() const {
  return _explicit_pages.size();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigPageManager::get_explicit_page
//       Access: Published
//  Description: Returns the nth explicit ConfigPage in the world.
//               See get_num_explicit_pages().
////////////////////////////////////////////////////////////////////
INLINE ConfigPage *ConfigPageManager::
get_explicit_page(int n) const {
  check_sort_pages();
  nassertr(n >= 0 && n < (int)_explicit_pages.size(), (ConfigPage *)NULL);
  return _explicit_pages[n];
}


////////////////////////////////////////////////////////////////////
//     Function: ConfigPageManager::mark_unsorted()
//       Access: Public
//  Description: This method is meant to be used internally to this
//               module; there is no need to call it directly.  It
//               indicates that the sort values of some pages may have
//               changed and pages need to be re-sorted.
////////////////////////////////////////////////////////////////////
INLINE void ConfigPageManager::
mark_unsorted() {
  _pages_sorted = false;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigPageManager::check_sort_pages()
//       Access: Private
//  Description: Called internally to ensure that the list of
//               pages is properly sorted.
////////////////////////////////////////////////////////////////////
INLINE void ConfigPageManager::
check_sort_pages() const {
  if (!_pages_sorted) {
    ((ConfigPageManager *)this)->sort_pages();
  }
}

INLINE ostream &
operator << (ostream &out, const ConfigPageManager &pageMgr) {
  pageMgr.output(out);
  return out;
}
