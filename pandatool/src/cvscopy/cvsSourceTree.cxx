// Filename: cvsSourceTree.cxx
// Created by:  drose (31Oct00)
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

#include "cvsSourceTree.h"
#include "cvsSourceDirectory.h"

#include "filename.h"
#include "executionEnvironment.h"
#include "notify.h"

#include <algorithm>
#include <ctype.h>
#include <stdio.h> // for perror
#include <errno.h>

#ifdef WIN32_VC
#include <direct.h>  // for chdir
#endif

bool CVSSourceTree::_got_start_fullpath = false;
Filename CVSSourceTree::_start_fullpath;

////////////////////////////////////////////////////////////////////
//     Function: CVSSourceTree::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CVSSourceTree::
CVSSourceTree() {
  _root = (CVSSourceDirectory *)NULL;
  _got_root_fullpath = false;
}

////////////////////////////////////////////////////////////////////
//     Function: CVSSourceTree::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CVSSourceTree::
~CVSSourceTree() {
  if (_root != (CVSSourceDirectory *)NULL) {
    delete _root;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CVSSourceTree::set_root
//       Access: Public
//  Description: Sets the root of the source directory.  This must be
//               called before scan(), and should not be called more
//               than once.
////////////////////////////////////////////////////////////////////
void CVSSourceTree::
set_root(const Filename &root_path) {
  nassertv(_path.empty());
  _path = root_path;
}

////////////////////////////////////////////////////////////////////
//     Function: CVSSourceTree::scan
//       Access: Public
//  Description: Scans the complete source directory starting at the
//               indicated pathname.  It is an error to call this more
//               than once.  Returns true on success, false if there
//               is an error.
////////////////////////////////////////////////////////////////////
bool CVSSourceTree::
scan(const Filename &key_filename) {
  nassertr(_root == (CVSSourceDirectory *)NULL, false);
  Filename root_fullpath = get_root_fullpath();
  _root = new CVSSourceDirectory(this, NULL, root_fullpath.get_basename());
  return _root->scan(_path, key_filename);
}

////////////////////////////////////////////////////////////////////
//     Function: CVSSourceTree::get_root
//       Access: Public
//  Description: Returns the root directory of the hierarchy.
////////////////////////////////////////////////////////////////////
CVSSourceDirectory *CVSSourceTree::
get_root() const {
  return _root;
}

////////////////////////////////////////////////////////////////////
//     Function: CVSSourceTree::find_directory
//       Access: Public
//  Description: Returns the source directory that corresponds to the
//               given path, or NULL if there is no such directory in
//               the source tree.
////////////////////////////////////////////////////////////////////
CVSSourceDirectory *CVSSourceTree::
find_directory(const Filename &path) {
  string root_fullpath = get_root_fullpath();
  string fullpath = get_actual_fullpath(path);

  // path is a subdirectory within the source hierarchy if and only if
  // root_fullpath is an initial prefix of fullpath.
  if (root_fullpath.length() > fullpath.length() ||
      fullpath.substr(0, root_fullpath.length()) != root_fullpath) {
    // Nope!
    return (CVSSourceDirectory *)NULL;
  }

  // The relative name is the part of fullpath not in root_fullpath.
  Filename relpath = fullpath.substr(root_fullpath.length());

  return _root->find_relpath(relpath);
}

////////////////////////////////////////////////////////////////////
//     Function: CVSSourceTree::find_relpath
//       Access: Public
//  Description: Returns the source directory that corresponds to the
//               given relative path from the root, or NULL if there
//               is no match.  The relative path may or may not
//               include the name of the root directory itself.
////////////////////////////////////////////////////////////////////
CVSSourceDirectory *CVSSourceTree::
find_relpath(const string &relpath) {
  CVSSourceDirectory *result = _root->find_relpath(relpath);
  if (result != (CVSSourceDirectory *)NULL) {
    return result;
  }

  // Check for the root dirname at the front of the path, and remove
  // it if it's there.
  size_t slash = relpath.find('/');
  Filename first = relpath.substr(0, slash);
  Filename rest;
  if (slash != string::npos) {
    rest = relpath.substr(slash + 1);
  }

  if (first == _root->get_dirname()) {
    return _root->find_relpath(rest);
  }

  return (CVSSourceDirectory *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CVSSourceTree::find_dirname
//       Access: Public
//  Description: Returns the source directory that corresponds to the
//               given local directory name, or NULL if there
//               is no match.
////////////////////////////////////////////////////////////////////
CVSSourceDirectory *CVSSourceTree::
find_dirname(const string &dirname) {
  return _root->find_dirname(dirname);
}

////////////////////////////////////////////////////////////////////
//     Function: CVSSourceTree::choose_directory
//       Access: Public
//  Description: Determines where an externally referenced model file
//               of the indicated name should go.  It does this by
//               looking for an existing model file of the same name;
//               if a matching model is not found, or if multiple
//               matching files are found, prompts the user for the
//               directory, or uses suggested_dir.
////////////////////////////////////////////////////////////////////
CVSSourceDirectory *CVSSourceTree::
choose_directory(const Filename &filename, CVSSourceDirectory *suggested_dir,
                 bool force, bool interactive) {
  static Directories empty_dirs;

  Filenames::const_iterator fi;
  fi = _filenames.find(filename);
  if (fi != _filenames.end()) {
    // The filename already exists somewhere.
    const Directories &dirs = (*fi).second;

    return prompt_user(filename, suggested_dir, dirs,
                       force, interactive);
  }

  // Now we have to prompt the user for a suitable place to put it.
  return prompt_user(filename, suggested_dir, empty_dirs,
                     force, interactive);
}

////////////////////////////////////////////////////////////////////
//     Function: CVSSourceTree::get_root_fullpath
//       Access: Public
//  Description: Returns the full path from the root to the top of
//               the source hierarchy.
////////////////////////////////////////////////////////////////////
Filename CVSSourceTree::
get_root_fullpath() {
  nassertr(!_path.empty(), Filename());
  if (!_got_root_fullpath) {
    _root_fullpath = get_actual_fullpath(_path);
    _got_root_fullpath = true;
  }
  return _root_fullpath;
}

////////////////////////////////////////////////////////////////////
//     Function: CVSSourceTree::get_root_dirname
//       Access: Public
//  Description: Returns the local directory name of the root of the
//               tree.
////////////////////////////////////////////////////////////////////
Filename CVSSourceTree::
get_root_dirname() const {
  nassertr(_root != (CVSSourceDirectory *)NULL, Filename());
  return _root->get_dirname();
}

////////////////////////////////////////////////////////////////////
//     Function: CVSSourceTree::add_file
//       Access: Public
//  Description: Adds a new file to the set of known files.  This is
//               normally called from CVSSourceDirectory::scan() and
//               should not be called directly by the user.
////////////////////////////////////////////////////////////////////
void CVSSourceTree::
add_file(const Filename &filename, CVSSourceDirectory *dir) {
  _filenames[filename].push_back(dir);
}

////////////////////////////////////////////////////////////////////
//     Function: CVSSourceTree::temp_chdir
//       Access: Public, Static
//  Description: Temporarily changes the current directory to the
//               named path.  Returns true on success, false on
//               failure.  Call restore_cwd() to restore to the
//               original directory later.
////////////////////////////////////////////////////////////////////
bool CVSSourceTree::
temp_chdir(const Filename &path) {
  // We have to call this first to guarantee that we have already
  // determined our starting directory.
  get_start_fullpath();

  string os_path = path.to_os_specific();
  if (chdir(os_path.c_str()) < 0) {
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CVSSourceTree::restore_cwd
//       Access: Public, Static
//  Description: Restores the current directory after changing it from
//               temp_chdir().
////////////////////////////////////////////////////////////////////
void CVSSourceTree::
restore_cwd() {
  Filename start_fullpath = get_start_fullpath();
  string os_path = start_fullpath.to_os_specific();

  if (chdir(os_path.c_str()) < 0) {
    // Hey!  We can't get back to the directory we started from!
    perror(os_path.c_str());
    nout << "Can't continue, aborting.\n";
    exit(1);
  }
}


////////////////////////////////////////////////////////////////////
//     Function: CVSSourceTree::prompt_user
//       Access: Private
//  Description: Prompts the user, if necessary, to choose a directory
//               to import the given file into.
////////////////////////////////////////////////////////////////////
CVSSourceDirectory *CVSSourceTree::
prompt_user(const string &filename, CVSSourceDirectory *suggested_dir,
            const CVSSourceTree::Directories &dirs,
            bool force, bool interactive) {
  if (dirs.size() == 1) {
    // The file already exists in exactly one place.
    if (!interactive) {
      return dirs[0];
    }
    CVSSourceDirectory *result = ask_existing(filename, dirs[0]);
    if (result != (CVSSourceDirectory *)NULL) {
      return result;
    }

  } else if (dirs.size() > 1) {
    // The file already exists in multiple places.
    if (force && !interactive) {
      return dirs[0];
    }
    CVSSourceDirectory *result = ask_existing(filename, dirs, suggested_dir);
    if (result != (CVSSourceDirectory *)NULL) {
      return result;
    }
  }

  // The file does not already exist, or the user declined to replace
  // an existing file.
  if (force && !interactive) {
    return suggested_dir;
  }

  if (find(dirs.begin(), dirs.end(), suggested_dir) == dirs.end()) {
    CVSSourceDirectory *result = ask_new(filename, suggested_dir);
    if (result != (CVSSourceDirectory *)NULL) {
      return result;
    }
  }

  // Ask the user where the damn thing should go.
  return ask_any(filename);
}

////////////////////////////////////////////////////////////////////
//     Function: CVSSourceTree::ask_existing
//       Access: Private
//  Description: Asks the user if he wants to replace an existing
//               file.
////////////////////////////////////////////////////////////////////
CVSSourceDirectory *CVSSourceTree::
ask_existing(const string &filename, CVSSourceDirectory *dir) {
  while (true) {
    nout << filename << " found in tree at "
         << dir->get_path() + "/" + filename << ".\n";
    string result = prompt("Overwrite this file (y/n)? ");
    nassertr(!result.empty(), (CVSSourceDirectory *)NULL);
    if (result.size() == 1) {
      if (tolower(result[0]) == 'y') {
        return dir;
      } else if (tolower(result[0]) == 'n') {
        return NULL;
      }
    }

    nout << "*** Invalid response: " << result << "\n\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CVSSourceTree::ask_existing
//       Access: Private
//  Description: Asks the user which of several existing files he
//               wants to replace.
////////////////////////////////////////////////////////////////////
CVSSourceDirectory *CVSSourceTree::
ask_existing(const string &filename, const CVSSourceTree::Directories &dirs,
             CVSSourceDirectory *suggested_dir) {
  while (true) {
    nout << filename << " found in tree at more than one place:\n";

    bool any_suggested = false;
    for (int i = 0; i < (int)dirs.size(); i++) {
      nout << "  " << (i + 1) << ". "
           << dirs[i]->get_path() + "/" + filename << "\n";
      if (dirs[i] == suggested_dir) {
        any_suggested = true;
      }
    }

    int next_option = dirs.size() + 1;
    int suggested_option = -1;

    if (!any_suggested) {
      suggested_option = next_option;
      next_option++;
      nout << "\n" << suggested_option
           << ". create "
           << suggested_dir->get_path() + "/" + filename
           << "\n";
    }

    int other_option = next_option;
    nout << other_option << ". Other\n";

    string result = prompt("Choose an option: ");
    nassertr(!result.empty(), (CVSSourceDirectory *)NULL);
    const char *nptr = result.c_str();
    char *endptr;
    int option = strtol(nptr, &endptr, 10);
    if (*endptr == '\0') {
      if (option >= 1 && option <= (int)dirs.size()) {
        return dirs[option - 1];

      } else if (option == suggested_option) {
        return suggested_dir;

      } else if (option == other_option) {
        return NULL;
      }
    }

    nout << "*** Invalid response: " << result << "\n\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CVSSourceTree::ask_new
//       Access: Private
//  Description: Asks the user if he wants to replace an existing
//               file.
////////////////////////////////////////////////////////////////////
CVSSourceDirectory *CVSSourceTree::
ask_new(const string &filename, CVSSourceDirectory *dir) {
  while (true) {
    nout << filename << " will be created in "
         << dir->get_path() << ".\n";
    string result = prompt("Create this file (y/n)? ");
    nassertr(!result.empty(), (CVSSourceDirectory *)NULL);
    if (result.size() == 1) {
      if (tolower(result[0]) == 'y') {
        return dir;
      } else if (tolower(result[0]) == 'n') {
        return NULL;
      }
    }

    nout << "*** Invalid response: " << result << "\n\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CVSSourceTree::ask_any
//       Access: Private
//  Description: Asks the user to type in the name of the directory in
//               which to store the file.
////////////////////////////////////////////////////////////////////
CVSSourceDirectory *CVSSourceTree::
ask_any(const string &filename) {
  while (true) {
    string result =
      prompt("Enter the name of the directory to copy " + filename + " to: ");
    nassertr(!result.empty(), (CVSSourceDirectory *)NULL);

    // The user might enter a fully-qualified path to the directory,
    // or a relative path from the root (with or without the root's
    // dirname), or the dirname of the particular directory.
    CVSSourceDirectory *dir = find_directory(result);
    if (dir == (CVSSourceDirectory *)NULL) {
      dir = find_relpath(result);
    }
    if (dir == (CVSSourceDirectory *)NULL) {
      dir = find_dirname(result);
    }

    if (dir != (CVSSourceDirectory *)NULL) {
      return dir;
    }

    nout << "*** Not a valid directory name: " << result << "\n\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CVSSourceTree::prompt
//       Access: Private
//  Description: Issues a prompt to the user and waits for a typed
//               response.  Returns the response (which will not be
//               empty).
////////////////////////////////////////////////////////////////////
string CVSSourceTree::
prompt(const string &message) {
  nout << flush;
  while (true) {
    cerr << message << flush;
    string response;
    getline(cin, response);

    // Remove leading and trailing whitespace.
    size_t p = 0;
    while (p < response.length() && isspace(response[p])) {
      p++;
    }

    size_t q = response.length();
    while (q > p && isspace(response[q - 1])) {
      q--;
    }

    if (q > p) {
      return response.substr(p, q - p);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CVSSourceTree::get_actual_fullpath
//       Access: Private, Static
//  Description: Determines the actual full path from the root to the
//               named directory.
////////////////////////////////////////////////////////////////////
Filename CVSSourceTree::
get_actual_fullpath(const Filename &path) {
  Filename canon = path;
  canon.make_canonical();
  return canon;
}


////////////////////////////////////////////////////////////////////
//     Function: CVSSourceTree::get_start_fullpath
//       Access: Private, Static
//  Description: Returns the full path from the root to the directory
//               in which the user started the program.
////////////////////////////////////////////////////////////////////
Filename CVSSourceTree::
get_start_fullpath() {
  if (!_got_start_fullpath) {
    Filename cwd = ExecutionEnvironment::get_cwd();
    _start_fullpath = cwd.to_os_specific();
  }
  return _start_fullpath;
}
