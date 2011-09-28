// Filename: dSearchPath.cxx
// Created by:  drose (01Jul00)
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

#include "dSearchPath.h"
#include "filename.h"

#include <algorithm>
#include <iterator>

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::Results::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
DSearchPath::Results::
Results() {
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::Results::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
DSearchPath::Results::
Results(const DSearchPath::Results &copy) :
  _files(copy._files)
{
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::Results::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void DSearchPath::Results::
operator = (const DSearchPath::Results &copy) {
  _files = copy._files;
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::Results::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
DSearchPath::Results::
~Results() {
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::Results::clear
//       Access: Published
//  Description: Removes all the files from the list.
////////////////////////////////////////////////////////////////////
void DSearchPath::Results::
clear() {
  _files.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::Results::get_num_files
//       Access: Published
//  Description: Returns the number of files on the result list.
////////////////////////////////////////////////////////////////////
int DSearchPath::Results::
get_num_files() const {
  return _files.size();
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::Results::get_file
//       Access: Published
//  Description: Returns the nth file on the result list.
////////////////////////////////////////////////////////////////////
const Filename &DSearchPath::Results::
get_file(int n) const {
  assert(n >= 0 && n < (int)_files.size());
  return _files[n];
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::Results::add_file
//       Access: Published
//  Description: Adds a new file to the result list.
////////////////////////////////////////////////////////////////////
void DSearchPath::Results::
add_file(const Filename &file) {
  _files.push_back(file);
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::Results::output
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void DSearchPath::Results::
output(ostream &out) const {
  out << "[ ";
  if (!_files.empty()) {
    Files::const_iterator fi = _files.begin();
    out << (*fi);
    ++fi;
    while (fi != _files.end()) {
      out << ", " << (*fi);
      ++fi;
    }
  }
  out << " ]";
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::Results::write
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void DSearchPath::Results::
write(ostream &out, int indent_level) const {
  Files::const_iterator fi;
  for (fi = _files.begin(); fi != _files.end(); ++fi) {
    for (int i = 0; i < indent_level; ++i) {
      out << ' ';
    }
    out << (*fi) << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::Default Constructor
//       Access: Published
//  Description: Creates an empty search path.
////////////////////////////////////////////////////////////////////
DSearchPath::
DSearchPath() {
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
DSearchPath::
DSearchPath(const string &path, const string &separator) {
  append_path(path, separator);
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
DSearchPath::
DSearchPath(const Filename &directory) {
  append_directory(directory);
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
DSearchPath::
DSearchPath(const DSearchPath &copy) :
  _directories(copy._directories)
{
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void DSearchPath::
operator = (const DSearchPath &copy) {
  _directories = copy._directories;
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
DSearchPath::
~DSearchPath() {
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::clear
//       Access: Published
//  Description: Removes all the directories from the search list.
////////////////////////////////////////////////////////////////////
void DSearchPath::
clear() {
  _directories.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::append_directory
//       Access: Published
//  Description: Adds a new directory to the end of the search list.
////////////////////////////////////////////////////////////////////
void DSearchPath::
append_directory(const Filename &directory) {
  _directories.push_back(directory);
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::prepend_directory
//       Access: Published
//  Description: Adds a new directory to the front of the search list.
////////////////////////////////////////////////////////////////////
void DSearchPath::
prepend_directory(const Filename &directory) {
  _directories.insert(_directories.begin(), directory);
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::append_path
//       Access: Published
//  Description: Adds all of the directories listed in the search path
//               to the end of the search list.
////////////////////////////////////////////////////////////////////
void DSearchPath::
append_path(const string &path, const string &separator) {
  string pathsep = separator;
  if (pathsep.empty()) {
    pathsep = DEFAULT_PATHSEP;
  }

  if (pathsep.empty()) {
    append_directory(path);

  } else {
    size_t p = 0;
    while (p < path.length()) {
      size_t q = path.find_first_of(pathsep, p);
      if (q == string::npos) {
        _directories.push_back(Filename::from_os_specific(path.substr(p)));
        return;
      }
      if (q != p) {
        _directories.push_back(Filename::from_os_specific(path.substr(p, q - p)));
      }
      p = q + 1;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::append_path
//       Access: Published
//  Description: Adds all of the directories listed in the search path
//               to the end of the search list.
////////////////////////////////////////////////////////////////////
void DSearchPath::
append_path(const DSearchPath &path) {
  copy(path._directories.begin(), path._directories.end(),
       back_inserter(_directories));
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::prepend_path
//       Access: Published
//  Description: Adds all of the directories listed in the search path
//               to the beginning of the search list.
////////////////////////////////////////////////////////////////////
void DSearchPath::
prepend_path(const DSearchPath &path) {
  if (!path._directories.empty()) {
    Directories new_directories = path._directories;
    copy(_directories.begin(), _directories.end(),
         back_inserter(new_directories));
    _directories.swap(new_directories);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::is_empty
//       Access: Published
//  Description: Returns true if the search list is empty, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool DSearchPath::
is_empty() const {
  return _directories.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::get_num_directories
//       Access: Published
//  Description: Returns the number of directories on the search list.
////////////////////////////////////////////////////////////////////
int DSearchPath::
get_num_directories() const {
  return _directories.size();
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::get_directory
//       Access: Published
//  Description: Returns the nth directory on the search list.
////////////////////////////////////////////////////////////////////
const Filename &DSearchPath::
get_directory(int n) const {
  assert(n >= 0 && n < (int)_directories.size());
  return _directories[n];
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::find_file
//       Access: Published
//  Description: Searches all the directories in the search list for
//               the indicated file, in order.  Returns the full
//               matching pathname of the first match if found, or the
//               empty string if not found.
////////////////////////////////////////////////////////////////////
Filename DSearchPath::
find_file(const Filename &filename) const {
  if (filename.is_local()) {
    if (_directories.empty()) {
      // Let's say an empty search path is the same as a search path
      // containing just ".".
      if (filename.exists()) {
        return filename;
      }

    } else {
      Directories::const_iterator di;
      for (di = _directories.begin(); di != _directories.end(); ++di) {
        Filename match((*di), filename);
        if (match.exists()) {
          if ((*di) == "." && filename.is_fully_qualified()) {
            // A special case for the "." directory: to avoid prefixing
            // an endless stream of ./ in front of files, if the
            // filename already has a ./ prefixed
            // (i.e. is_fully_qualified() is true), we don't
            // prefix another one.
            return filename;
          } else {
            return match;
          }
        }
      }
    }
  }

  return string();
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::find_all_files
//       Access: Published
//  Description: Searches all the directories in the search list for
//               the indicated file, in order.  Fills up the results
//               list with *all* of the matching filenames found, if
//               any.  Returns the number of matches found.
//
//               It is the responsibility of the the caller to clear
//               the results list first; otherwise, the newly-found
//               files will be appended to the list.
////////////////////////////////////////////////////////////////////
int DSearchPath::
find_all_files(const Filename &filename,
               DSearchPath::Results &results) const {
  int num_added = 0;

  if (filename.is_local()) {
    if (_directories.empty()) {
      // Let's say an empty search path is the same as a search path
      // containing just ".".
      if (filename.exists()) {
        results.add_file(filename);
      }

    } else {
      Directories::const_iterator di;
      for (di = _directories.begin(); di != _directories.end(); ++di) {
        Filename match((*di), filename);
        if (match.exists()) {
          if ((*di) == "." && filename.is_fully_qualified()) {
            // A special case for the "." directory: to avoid prefixing
            // an endless stream of ./ in front of files, if the
            // filename already has a ./ prefixed
            // (i.e. is_fully_qualified() is true), we don't
            // prefix another one.
            results.add_file(filename);
          } else {
            results.add_file(match);
          }
          num_added++;
        }
      }
    }
  }

  return num_added;
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::output
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void DSearchPath::
output(ostream &out, const string &separator) const {
  string pathsep = separator;
  if (pathsep.empty()) {
    pathsep = DEFAULT_PATHSEP;
    if (!pathsep.empty()) {
      pathsep = pathsep[0];
    }
  }

  if (!_directories.empty()) {
    Directories::const_iterator di = _directories.begin();
    out << (*di);
    ++di;
    while (di != _directories.end()) {
      out << pathsep << (*di);
      ++di;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::write
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void DSearchPath::
write(ostream &out, int indent_level) const {
  Directories::const_iterator di;
  for (di = _directories.begin(); di != _directories.end(); ++di) {
    for (int i = 0; i < indent_level; ++i) {
      out << ' ';
    }
    out << (*di) << "\n";
  }
}


