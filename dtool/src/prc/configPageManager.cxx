// Filename: configPageManager.cxx
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

#include "configPageManager.h"
#include "configDeclaration.h"
#include "configPage.h"
#include "prcKeyRegistry.h"
#include "dSearchPath.h"
#include "executionEnvironment.h"
#include "pset.h"
#include "config_prc.h"
#include "pfstream.h"

// Pick up the public key definitions.
#ifdef PRC_PUBLIC_KEYS_INCLUDE
#include PRC_PUBLIC_KEYS_INCLUDE
#endif

#include <algorithm>
#include <ctype.h>

ConfigPageManager *ConfigPageManager::_global_ptr = NULL;

////////////////////////////////////////////////////////////////////
//     Function: ConfigPageManager::Constructor
//       Access: Protected
//  Description: The constructor is private (actually, just protected,
//               but only to avoid a gcc compiler warning) because it
//               should not be explicitly constructed.  There is only
//               one ConfigPageManager, and it constructs itself.
////////////////////////////////////////////////////////////////////
ConfigPageManager::
ConfigPageManager() {
  _next_page_seq = 1;
  _loaded_implicit = false;
  _currently_loading = false;
  _pages_sorted = true;

#ifdef PRC_PUBLIC_KEYS_INCLUDE
  // Record the public keys in the registry at startup time.
  PrcKeyRegistry::get_global_ptr()->record_keys(prc_pubkeys, num_prc_pubkeys);
#endif  // PRC_PUBLIC_KEYS_INCLUDE
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigPageManager::Destructor
//       Access: Protected
//  Description: The ConfigPageManager destructor should never be
//               called, because this is a global object that is never
//               freed.
////////////////////////////////////////////////////////////////////
ConfigPageManager::
~ConfigPageManager() {
  prc_cat->error()
    << "Internal error--ConfigPageManager destructor called!\n";
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigPageManager::reload_implicit_pages
//       Access: Published
//  Description: Searches the PRC_DIR and/or PRC_PATH directories for
//               *.prc files and loads them in as pages.
//
//               This may be called after startup, to force the system
//               to re-read all of the implicit prc files.
////////////////////////////////////////////////////////////////////
void ConfigPageManager::
reload_implicit_pages() {
  if (_currently_loading) {
    // This is a recursion protector.  We can get recursion feedback
    // between config and notify, as each tries to use the other at
    // construction.
    return;
  }
  _currently_loading = true;

  // First, remove all the previously-loaded pages.
  Pages::iterator pi;
  for (pi = _implicit_pages.begin(); pi != _implicit_pages.end(); ++pi) {
    delete (*pi);
  }
  _implicit_pages.clear();

  // PRC_PATTERNS lists one or more filename templates separated by
  // spaces.  Pull them out and store them in _prc_patterns.
  _prc_patterns.clear();

  string prc_patterns = PRC_PATTERNS;
  if (!prc_patterns.empty()) {
    vector_string pat_list;
    ConfigDeclaration::extract_words(prc_patterns, pat_list);
    _prc_patterns.reserve(pat_list.size());
    for (size_t i = 0; i < pat_list.size(); ++i) {
      GlobPattern glob(pat_list[i]);
#ifdef WIN32
      // On windows, the file system is case-insensitive, so the
      // pattern should be too.
      glob.set_case_sensitive(false);
#endif  // WIN32
      _prc_patterns.push_back(glob);
    }
  }

  // Similarly for PRC_EXECUTABLE_PATTERNS.
  _prc_executable_patterns.clear();

  string prc_executable_patterns = PRC_EXECUTABLE_PATTERNS;
  if (!prc_executable_patterns.empty()) {
    vector_string pat_list;
    ConfigDeclaration::extract_words(prc_executable_patterns, pat_list);
    _prc_executable_patterns.reserve(pat_list.size());
    for (size_t i = 0; i < pat_list.size(); ++i) {
      GlobPattern glob(pat_list[i]);
#ifdef WIN32
      glob.set_case_sensitive(false);
#endif  // WIN32
      _prc_executable_patterns.push_back(glob);
    }
  }

  // Now build up the search path for .prc files.
  _search_path.clear();

  // PRC_DIR_ENVVARS lists one or more environment variables separated
  // by spaces.  Pull them out, and each of those contains the name of
  // a single directory to search.  Add it to the search path.
  string prc_dir_envvars = PRC_DIR_ENVVARS;
  if (!prc_dir_envvars.empty()) {
    vector_string prc_dir_envvar_list;
    ConfigDeclaration::extract_words(prc_dir_envvars, prc_dir_envvar_list);
    for (size_t i = 0; i < prc_dir_envvar_list.size(); ++i) {
      string prc_dir = ExecutionEnvironment::get_environment_variable(prc_dir_envvar_list[i]);
      if (!prc_dir.empty()) {
        Filename prc_dir_filename = Filename::from_os_specific(prc_dir);
        if (scan_auto_prc_dir(prc_dir_filename)) {
          _search_path.append_directory(prc_dir_filename);
        }
      }
    }
  }

  // PRC_PATH_ENVVARS lists one or more environment variables separated
  // by spaces.  Pull them out, and then each one of those contains a
  // list of directories to search.  Add each of those to the search
  // path.
  string prc_path_envvars = PRC_PATH_ENVVARS;
  if (!prc_path_envvars.empty()) {
    vector_string prc_path_envvar_list;
    ConfigDeclaration::extract_words(prc_path_envvars, prc_path_envvar_list);
    for (size_t i = 0; i < prc_path_envvar_list.size(); ++i) {
      string prc_path = ExecutionEnvironment::get_environment_variable(prc_path_envvar_list[i]);
      if (!prc_path.empty()) {
        _search_path.append_path(prc_path);
      }
    }
  }

  if (_search_path.is_empty()) {
    // If nothing's on the search path (PRC_DIR and PRC_PATH were not
    // defined), then use the DEFAULT_PRC_DIR.
    string default_prc_dir = DEFAULT_PRC_DIR;
    if (!default_prc_dir.empty()) {
      // It's already from-os-specific by ppremake.
      Filename prc_dir_filename = default_prc_dir;
      if (scan_auto_prc_dir(prc_dir_filename)) {
        _search_path.append_directory(default_prc_dir);
      }
    }
  }

  // Now find all of the *.prc files (or whatever matches
  // PRC_PATTERNS) on the path.
  ConfigFiles config_files;

  // Use a pset to ensure that we only visit each directory once, even
  // if it appears multiple times (under different aliases!) in the
  // path.
  pset<Filename> unique_dirnames;

  // We walk through the list of directories in forward order, so that
  // the most important directories are visited first.
  for (int di = 0; di < _search_path.get_num_directories(); ++di) {
    const Filename &directory = _search_path.get_directory(di);
    if (directory.is_directory()) {
      Filename canonical(directory, ".");
      canonical.make_canonical();
      if (unique_dirnames.insert(canonical).second) {
        vector_string files;
        directory.scan_directory(files);

        // We walk through the directory's list of files in reverse
        // alphabetical order, because for historical reasons, the
        // most important file within a directory is the
        // alphabetically last file of that directory, and we still
        // want to visit the most important files first.
        vector_string::reverse_iterator fi;
        for (fi = files.rbegin(); fi != files.rend(); ++fi) {
          int file_flags = 0;
          Globs::const_iterator gi;
          for (gi = _prc_patterns.begin();
               gi != _prc_patterns.end();
               ++gi) {
            if ((*gi).matches(*fi)) {
              file_flags |= FF_read;
              break;
            }
          }
          for (gi = _prc_executable_patterns.begin();
               gi != _prc_executable_patterns.end();
               ++gi) {
            if ((*gi).matches(*fi)) {
              file_flags |= FF_execute;
              break;
            }
          }
          if (file_flags != 0) {
            ConfigFile file;
            file._file_flags = file_flags;
            file._filename = Filename(directory, (*fi));
            config_files.push_back(file);
          }
        }
      }
    }
  }

  // Now we have a list of filenames in order from most important to
  // least important.  Walk through the list in reverse order to load
  // their contents, because we want the first file in the list (the
  // most important) to be on the top of the stack.
  ConfigFiles::reverse_iterator ci;
  int i = 1;
  for (ci = config_files.rbegin(); ci != config_files.rend(); ++ci) {
    const ConfigFile &file = (*ci);
    Filename filename = file._filename;

    if ((file._file_flags & FF_execute) != 0 &&
        filename.is_executable()) {
      // Attempt to execute the file as a command.
      string command = filename.to_os_specific();

      string envvar = PRC_EXECUTABLE_ARGS_ENVVAR;
      if (!envvar.empty()) {
        string args = ExecutionEnvironment::get_environment_variable(envvar);
        if (!args.empty()) {
          command += " ";
          command += args;
        }
      }
      IPipeStream ifs(command);

      ConfigPage *page = new ConfigPage(filename, true, i);
      ++i;
      _implicit_pages.push_back(page);
      _pages_sorted = false;
      
      page->read_prc(ifs);
    } else if ((file._file_flags & FF_read) != 0) {
      // Just read the file.
      filename.set_text();
      
      ifstream in;
      if (!filename.open_read(in)) {
        prc_cat.error()
          << "Unable to read " << filename << "\n";
      } else {
        ConfigPage *page = new ConfigPage(filename, true, i);
        ++i;
        _implicit_pages.push_back(page);
        _pages_sorted = false;
        
        page->read_prc(in);
      }
    }
  }

  if (!_loaded_implicit) {
    Notify::ptr()->config_initialized();
    _loaded_implicit = true;
  }

  _currently_loading = false;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigPageManager::make_explicit_page
//       Access: Published
//  Description: Creates and returns a new, empty ConfigPage.  This
//               page will be stacked on top of any pages that were
//               created before; it may shadow variable declarations
//               that are defined in previous pages.
////////////////////////////////////////////////////////////////////
ConfigPage *ConfigPageManager::
make_explicit_page(const string &name) {
  ConfigPage *page = new ConfigPage(name, false, _next_page_seq);
  ++_next_page_seq;
  _explicit_pages.push_back(page);
  _pages_sorted = false;
  return page;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigPageManager::delete_explicit_page
//       Access: Published
//  Description: Removes a previously-constructed ConfigPage from the
//               set of active pages, and deletes it.  The ConfigPage
//               object is no longer valid after this call.  Returns
//               true if the page is successfully deleted, or false if
//               it was unknown (which should never happen if the page
//               was legitimately constructed).
////////////////////////////////////////////////////////////////////
bool ConfigPageManager::
delete_explicit_page(ConfigPage *page) {
  Pages::iterator pi;
  for (pi = _explicit_pages.begin(); pi != _explicit_pages.end(); ++pi) {
    if ((*pi) == page) {
      _explicit_pages.erase(pi);
      delete page;
      return true;
    }
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigPageManager::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void ConfigPageManager::
output(ostream &out) const {
  out << "ConfigPageManager, " 
      << _explicit_pages.size() + _implicit_pages.size() 
      << " pages.";
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigPageManager::write
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void ConfigPageManager::
write(ostream &out) const {
  check_sort_pages();
  out << _explicit_pages.size() << " explicit pages:\n";

  Pages::const_iterator pi;
  for (pi = _explicit_pages.begin(); pi != _explicit_pages.end(); ++pi) {
    const ConfigPage *page = (*pi);
    out << "  " << page->get_name();
    if (page->get_trust_level() > 0) {
      out << "  (signed " << page->get_trust_level() << ")\n";
    } else if (!page->get_signature().empty()) {
      out << "  (invalid signature)\n";
    } else {
      out << "\n";
    }
  }

  out << "\n" << _implicit_pages.size() << " implicit pages:\n";
  for (pi = _implicit_pages.begin(); pi != _implicit_pages.end(); ++pi) {
    const ConfigPage *page = (*pi);
    out << "  " << page->get_name();
    if (page->get_trust_level() > 0) {
      out << "  (signed " << page->get_trust_level() << ")\n";
    } else if (!page->get_signature().empty()) {
      out << "  (invalid signature)\n";
    } else {
      out << "\n";
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigPageManager::get_global_ptr
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
ConfigPageManager *ConfigPageManager::
get_global_ptr() {
  if (_global_ptr == (ConfigPageManager *)NULL) {
    _global_ptr = new ConfigPageManager;
  }
  return _global_ptr;
}

// This class is used in sort_pages, below.
class CompareConfigPages {
public:
  bool operator () (const ConfigPage *a, const ConfigPage *b) const {
    return (*a) < (*b);
  }
};

////////////////////////////////////////////////////////////////////
//     Function: ConfigPageManager::sort_pages
//       Access: Private
//  Description: Sorts the list of pages into priority order,
//               so that the page at the front of the list is
//               the one that shadows all following pages.
////////////////////////////////////////////////////////////////////
void ConfigPageManager::
sort_pages() {
  sort(_implicit_pages.begin(), _implicit_pages.end(), CompareConfigPages());
  sort(_explicit_pages.begin(), _explicit_pages.end(), CompareConfigPages());

  _pages_sorted = true;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigPageManager::scan_auto_prc_dir
//       Access: Private
//  Description: Checks for the prefix "<auto>" in the value of the
//               $PRC_DIR environment variable (or in the compiled-in
//               DEFAULT_PRC_DIR value).  If it is found, then the
//               actual directory is determined by searching upward
//               from the executable's starting directory, or from the
//               current working directory, until at least one .prc
//               file is found.
//
//               Returns true if the prc_dir has been filled with a
//               valid directory name, false if no good directory name
//               was found.
////////////////////////////////////////////////////////////////////
bool ConfigPageManager::
scan_auto_prc_dir(Filename &prc_dir) const {
  string prc_dir_string = prc_dir;
  if (prc_dir_string.substr(0, 6) == "<auto>") {
    Filename suffix = prc_dir_string.substr(6);
    
    // Start at the executable directory.
    Filename binary = ExecutionEnvironment::get_binary_name();
    Filename dir = binary.get_dirname();

    if (scan_up_from(prc_dir, dir, suffix)) {
      return true;
    }
    
    // Try the current working directory.
    dir = ExecutionEnvironment::get_cwd();
    if (scan_up_from(prc_dir, dir, suffix)) {
      return true;
    }

    // Didn't find it; too bad.
    cerr << "Warning: unable to auto-locate config files in directory named by \""
         << prc_dir << "\".\n";
    return false;
  }

  // The filename did not begin with "<auto>", so it stands unchanged.
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigPageManager::scan_up_from
//       Access: Private
//  Description: Used to implement scan_auto_prc_dir(), above, this
//               scans upward from the indicated directory name until
//               a directory is found that includes at least one .prc
//               file, or the root directory is reached.  
//
//               If a match is found, puts it result and returns true;
//               otherwise, returns false.
////////////////////////////////////////////////////////////////////
bool ConfigPageManager::
scan_up_from(Filename &result, const Filename &dir, 
             const Filename &suffix) const {
  Filename consider(dir, suffix);

  vector_string files;
  if (consider.scan_directory(files)) {
    vector_string::const_iterator fi;
    for (fi = files.begin(); fi != files.end(); ++fi) {
      Globs::const_iterator gi;
      for (gi = _prc_patterns.begin();
           gi != _prc_patterns.end();
           ++gi) {
        if ((*gi).matches(*fi)) {
          result = consider;
          return true;
        }
      }
      
      for (gi = _prc_executable_patterns.begin();
           gi != _prc_executable_patterns.end();
           ++gi) {
        if ((*gi).matches(*fi)) {
          result = consider;
          return true;
        }
      }
    }
  }

  Filename parent = dir.get_dirname();

  if (dir == parent) {
    // Too bad; couldn't find a match.
    return false;
  }

  // Recursively try again on the parent.
  return scan_up_from(result, parent, suffix);
}
