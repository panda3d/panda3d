// Filename: cvsSourceTree.cxx
// Created by:  drose (31Oct00)
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

#include "cvsSourceTree.h"
#include "cvsSourceDirectory.h"

#include "filename.h"
#include "executionEnvironment.h"
#include "pnotify.h"
#include "string_utils.h"

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
      cmp_nocase(fullpath.substr(0, root_fullpath.length()), root_fullpath) != 0) {
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

  if (cmp_nocase(first, _root->get_dirname()) == 0) {
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
CVSSourceTree::FilePath CVSSourceTree::
choose_directory(const string &basename, CVSSourceDirectory *suggested_dir,
                 bool force, bool interactive) {
  static FilePaths empty_paths;

  Basenames::const_iterator bi;
  bi = _basenames.find(downcase(basename));
  if (bi != _basenames.end()) {
    // The filename already exists somewhere.
    const FilePaths &paths = (*bi).second;

    return prompt_user(basename, suggested_dir, paths,
                       force, interactive);
  }

  // Now we have to prompt the user for a suitable place to put it.
  return prompt_user(basename, suggested_dir, empty_paths,
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
add_file(const string &basename, CVSSourceDirectory *dir) {
  FilePath file_path(dir, basename);
  _basenames[downcase(basename)].push_back(file_path);
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
CVSSourceTree::FilePath CVSSourceTree::
prompt_user(const string &basename, CVSSourceDirectory *suggested_dir,
            const CVSSourceTree::FilePaths &paths,
            bool force, bool interactive) {
  if (paths.size() == 1) {
    // The file already exists in exactly one place.
    if (!interactive) {
      return paths[0];
    }
    FilePath result = ask_existing(basename, paths[0]);
    if (result.is_valid()) {
      return result;
    }

  } else if (paths.size() > 1) {
    // The file already exists in multiple places.
    if (force && !interactive) {
      return paths[0];
    }
    FilePath result = ask_existing(basename, paths, suggested_dir);
    if (result.is_valid()) {
      return result;
    }
  }

  // The file does not already exist, or the user declined to replace
  // an existing file.
  if (force && !interactive) {
    return FilePath(suggested_dir, basename);
  }

  // Is the file already in the suggested directory?  If not, prompt
  // the user to put it there.
  bool found_dir = false;
  FilePaths::const_iterator pi;
  for (pi = paths.begin(); pi != paths.end(); ++pi) {
    if ((*pi)._dir == suggested_dir) {
      found_dir = true;
      break;
    }
  }

  if (!found_dir) {
    FilePath result = ask_new(basename, suggested_dir);
    if (result.is_valid()) {
      return result;
    }
  }

  // Ask the user where the damn thing should go.
  return ask_any(basename, paths);
}

////////////////////////////////////////////////////////////////////
//     Function: CVSSourceTree::ask_existing
//       Access: Private
//  Description: Asks the user if he wants to replace an existing
//               file.
////////////////////////////////////////////////////////////////////
CVSSourceTree::FilePath CVSSourceTree::
ask_existing(const string &basename, const CVSSourceTree::FilePath &path) {
  while (true) {
    nout << basename << " found in tree at "
         << path.get_path() << ".\n";
    string result = prompt("Overwrite this file (y/n)? ");
    nassertr(!result.empty(), FilePath());
    if (result.size() == 1) {
      if (tolower(result[0]) == 'y') {
        return path;
      } else if (tolower(result[0]) == 'n') {
        return FilePath();
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
CVSSourceTree::FilePath CVSSourceTree::
ask_existing(const string &basename, const CVSSourceTree::FilePaths &paths,
             CVSSourceDirectory *suggested_dir) {
  while (true) {
    nout << basename << " found in tree at more than one place:\n";

    bool any_suggested = false;
    for (int i = 0; i < (int)paths.size(); i++) {
      nout << "  " << (i + 1) << ". "
           << paths[i].get_path() << "\n";
      if (paths[i]._dir == suggested_dir) {
        any_suggested = true;
      }
    }

    int next_option = paths.size() + 1;
    int suggested_option = -1;

    if (!any_suggested) {
      // If it wasn't already in the suggested directory, offer to put
      // it there.
      suggested_option = next_option;
      next_option++;
      nout << "\n" << suggested_option
           << ". create "
           << Filename(suggested_dir->get_path(), basename)
           << "\n";
    }

    int other_option = next_option;
    nout << other_option << ". Other\n";

    string result = prompt("Choose an option: ");
    nassertr(!result.empty(), FilePath());
    const char *nptr = result.c_str();
    char *endptr;
    int option = strtol(nptr, &endptr, 10);
    if (*endptr == '\0') {
      if (option >= 1 && option <= (int)paths.size()) {
        return paths[option - 1];

      } else if (option == suggested_option) {
        return FilePath(suggested_dir, basename);

      } else if (option == other_option) {
        return FilePath();
      }
    }

    nout << "*** Invalid response: " << result << "\n\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CVSSourceTree::ask_new
//       Access: Private
//  Description: Asks the user if he wants to create a new file.
////////////////////////////////////////////////////////////////////
CVSSourceTree::FilePath CVSSourceTree::
ask_new(const string &basename, CVSSourceDirectory *dir) {
  while (true) {
    nout << basename << " will be created in "
         << dir->get_path() << ".\n";
    string result = prompt("Create this file (y/n)? ");
    nassertr(!result.empty(), FilePath());
    if (result.size() == 1) {
      if (tolower(result[0]) == 'y') {
        return FilePath(dir, basename);
      } else if (tolower(result[0]) == 'n') {
        return FilePath();
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
CVSSourceTree::FilePath CVSSourceTree::
ask_any(const string &basename,
        const CVSSourceTree::FilePaths &paths) {
  while (true) {
    string result =
      prompt("Enter the name of the directory to copy " + basename + " to: ");
    nassertr(!result.empty(), FilePath());

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
      // If the file is already in this directory, we must preserve
      // its existing case.
      FilePaths::const_iterator pi;
      for (pi = paths.begin(); pi != paths.end(); ++pi) {
        if ((*pi)._dir == dir) {
          return (*pi);
        }
      }

      // Otherwise, since we're creating a new file, keep the original
      // case.
      return FilePath(dir, basename);
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


////////////////////////////////////////////////////////////////////
//     Function: CVSSourceTree::FilePath::Constructor
//       Access: Public
//  Description: Creates an invalid FilePath specification.
////////////////////////////////////////////////////////////////////
CVSSourceTree::FilePath::
FilePath() :
  _dir(NULL)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CVSSourceTree::FilePath::Constructor
//       Access: Public
//  Description: Creates a valid FilePath specification with the
//               indicated directory and basename.
////////////////////////////////////////////////////////////////////
CVSSourceTree::FilePath::
FilePath(CVSSourceDirectory *dir, const string &basename) :
  _dir(dir),
  _basename(basename)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CVSSourceTree::FilePath::is_valid
//       Access: Public
//  Description: Returns true if this FilePath represents a valid
//               file, or false if it represents an error return.
////////////////////////////////////////////////////////////////////
bool CVSSourceTree::FilePath::
is_valid() const {
  return (_dir != (CVSSourceDirectory *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: CVSSourceTree::FilePath::get_path
//       Access: Public
//  Description: Returns the relative path to this file from the root
//               of the source tree.
////////////////////////////////////////////////////////////////////
Filename CVSSourceTree::FilePath::
get_path() const {
  nassertr(_dir != (CVSSourceDirectory *)NULL, Filename());
  return Filename(_dir->get_path(), _basename);
}

////////////////////////////////////////////////////////////////////
//     Function: CVSSourceTree::FilePath::get_fullpath
//       Access: Public
//  Description: Returns the full path to this file.
////////////////////////////////////////////////////////////////////
Filename CVSSourceTree::FilePath::
get_fullpath() const {
  nassertr(_dir != (CVSSourceDirectory *)NULL, Filename());
  return Filename(_dir->get_fullpath(), _basename);
}

////////////////////////////////////////////////////////////////////
//     Function: CVSSourceTree::FilePath::get_rel_from
//       Access: Public
//  Description: Returns the relative path to this file as seen from
//               the indicated source directory.
////////////////////////////////////////////////////////////////////
Filename CVSSourceTree::FilePath::
get_rel_from(const CVSSourceDirectory *other) const {
  nassertr(_dir != (CVSSourceDirectory *)NULL, Filename());
  return Filename(other->get_rel_to(_dir), _basename);
}

