// Filename: configPageManager.h
// Created by:  drose (15Oct04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef CONFIGPAGEMANAGER_H
#define CONFIGPAGEMANAGER_H

#include "dtoolbase.h"
#include "pvector.h"
#include "dSearchPath.h"
#include "globPattern.h"
#include "notify.h"

class ConfigPage;

////////////////////////////////////////////////////////////////////
//       Class : ConfigPageManager
// Description : A global object that maintains the set of ConfigPages
//               everywhere in the world, and keeps them in sorted
//               order.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOLCONFIG ConfigPageManager {
protected:
  ConfigPageManager();
  ~ConfigPageManager();

PUBLISHED:
  INLINE bool loaded_implicit_pages() const;
  INLINE void load_implicit_pages();
  void reload_implicit_pages();

  INLINE DSearchPath &get_search_path();

  INLINE int get_num_prc_patterns() const;
  INLINE string get_prc_pattern(int n) const;

  INLINE int get_num_prc_executable_patterns() const;
  INLINE string get_prc_executable_pattern(int n) const;

  ConfigPage *make_explicit_page(const string &name);
  bool delete_explicit_page(ConfigPage *page);

  INLINE int get_num_implicit_pages() const;
  INLINE ConfigPage *get_implicit_page(int n) const;

  INLINE int get_num_explicit_pages() const;
  INLINE ConfigPage *get_explicit_page(int n) const;

  void output(ostream &out) const;
  void write(ostream &out) const;

  static ConfigPageManager *get_global_ptr();

private:
  INLINE void check_sort_pages() const;
  void sort_pages();

  typedef pvector<ConfigPage *> Pages;
  Pages _implicit_pages;
  Pages _explicit_pages;
  bool _pages_sorted;
  int _next_page_seq;

  bool _loaded_implicit;
  bool _currently_loading;

  DSearchPath _search_path;

  typedef pvector<GlobPattern> Globs;
  Globs _prc_patterns;
  Globs _prc_executable_patterns;

  // In load_implicit_pages(), we temporarily build up a list of
  // potential config files to read and/or execute.  We'll need some
  // data structures to store that information.
  enum FileFlags {
    FF_read     = 0x001,
    FF_execute  = 0x002,
  };
  class ConfigFile {
  public:
    int _file_flags;
    Filename _filename;
  };
  typedef pvector<ConfigFile> ConfigFiles;

  static ConfigPageManager *_global_ptr;
};

INLINE ostream &operator << (ostream &out, const ConfigPageManager &pageMgr);

#include "configPageManager.I"

#endif
