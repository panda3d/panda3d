// Filename: configPageManager.h
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

#ifndef CONFIGPAGEMANAGER_H
#define CONFIGPAGEMANAGER_H

#include "dtoolbase.h"
#include "configFlags.h"
#include "dSearchPath.h"
#include "globPattern.h"
#include "pnotify.h"

#include <vector>

class ConfigPage;

////////////////////////////////////////////////////////////////////
//       Class : ConfigPageManager
// Description : A global object that maintains the set of ConfigPages
//               everywhere in the world, and keeps them in sorted
//               order.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOLCONFIG ConfigPageManager : public ConfigFlags {
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

  INLINE int get_num_prc_encrypted_patterns() const;
  INLINE string get_prc_encrypted_pattern(int n) const;

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

public:
  INLINE void mark_unsorted();

private:
  INLINE void check_sort_pages() const;
  void sort_pages();

  bool scan_auto_prc_dir(Filename &prc_dir) const;
  bool scan_up_from(Filename &result, const Filename &dir, 
                    const Filename &suffix) const;

  void config_initialized();

  typedef vector<ConfigPage *> Pages;
  Pages _implicit_pages;
  Pages _explicit_pages;
  bool _pages_sorted;
  int _next_page_seq;

  bool _loaded_implicit;
  bool _currently_loading;

  DSearchPath _search_path;

  typedef vector<GlobPattern> Globs;
  Globs _prc_patterns;
  Globs _prc_encrypted_patterns;
  Globs _prc_executable_patterns;

  // In load_implicit_pages(), we temporarily build up a list of
  // potential config files to read and/or execute.  We'll need some
  // data structures to store that information.
  enum FileFlags {
    FF_read     = 0x001,
    FF_execute  = 0x002,
    FF_decrypt  = 0x004,
  };
  class ConfigFile {
  public:
    int _file_flags;
    Filename _filename;
  };
  typedef vector<ConfigFile> ConfigFiles;

  static ConfigPageManager *_global_ptr;
};

INLINE ostream &operator << (ostream &out, const ConfigPageManager &pageMgr);

#include "configPageManager.I"

#endif
