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
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DSearchPath::Results::
Results() {
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::Results::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DSearchPath::Results::
Results(const DSearchPath::Results &copy) :
  _files(copy._files)
{
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::Results::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void DSearchPath::Results::
operator = (const DSearchPath::Results &copy) {
  _files = copy._files;
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::Results::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DSearchPath::Results::
~Results() {
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::Results::clear
//       Access: Public
//  Description: Removes all the files from the list.
////////////////////////////////////////////////////////////////////
void DSearchPath::Results::
clear() {
  _files.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::Results::get_num_files
//       Access: Public
//  Description: Returns the number of files on the result list.
////////////////////////////////////////////////////////////////////
int DSearchPath::Results::
get_num_files() const {
  return _files.size();
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::Results::get_file
//       Access: Public
//  Description: Returns the nth file on the result list.
////////////////////////////////////////////////////////////////////
const Filename &DSearchPath::Results::
get_file(int n) const {
  assert(n >= 0 && n < (int)_files.size());
  return _files[n];
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::Results::add_file
//       Access: Public
//  Description: Adds a new file to the result list.
////////////////////////////////////////////////////////////////////
void DSearchPath::Results::
add_file(const Filename &file) {
  _files.push_back(file);
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::Default Constructor
//       Access: Public
//  Description: Creates an empty search path.
////////////////////////////////////////////////////////////////////
DSearchPath::
DSearchPath() {
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DSearchPath::
DSearchPath(const string &path, const string &delimiters) {
  append_path(path, delimiters);
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DSearchPath::
DSearchPath(const DSearchPath &copy) :
  _directories(copy._directories)
{
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void DSearchPath::
operator = (const DSearchPath &copy) {
  _directories = copy._directories;
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DSearchPath::
~DSearchPath() {
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::clear
//       Access: Public
//  Description: Removes all the directories from the search list.
////////////////////////////////////////////////////////////////////
void DSearchPath::
clear() {
  _directories.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::append_directory
//       Access: Public
//  Description: Adds a new directory to the end of the search list.
////////////////////////////////////////////////////////////////////
void DSearchPath::
append_directory(const Filename &directory) {
  _directories.push_back(directory);
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::prepend_directory
//       Access: Public
//  Description: Adds a new directory to the front of the search list.
////////////////////////////////////////////////////////////////////
void DSearchPath::
prepend_directory(const Filename &directory) {
  _directories.insert(_directories.begin(), directory);
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::append_path
//       Access: Public
//  Description: Adds all of the directories listed in the search path
//               to the end of the search list.
////////////////////////////////////////////////////////////////////
void DSearchPath::
append_path(const string &path, const string &delimiters) {
  size_t p = 0;
  while (p < path.length()) {
    size_t q = path.find_first_of(delimiters, p);
    if (q == string::npos) {
      _directories.push_back(path.substr(p));
      return;
    }
    if (q != p) {
      _directories.push_back(path.substr(p, q - p));
    }
    p = q + 1;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::append_path
//       Access: Public
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
//       Access: Public
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
//       Access: Public
//  Description: Returns true if the search list is empty, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool DSearchPath::
is_empty() const {
  return _directories.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::get_num_directories
//       Access: Public
//  Description: Returns the number of directories on the search list.
////////////////////////////////////////////////////////////////////
int DSearchPath::
get_num_directories() const {
  return _directories.size();
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::get_directory
//       Access: Public
//  Description: Returns the nth directory on the search list.
////////////////////////////////////////////////////////////////////
const Filename &DSearchPath::
get_directory(int n) const {
  assert(n >= 0 && n < (int)_directories.size());
  return _directories[n];
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::find_file
//       Access: Public
//  Description: Searches all the directories in the search list for
//               the indicated file, in order.  Returns the full
//               matching pathname of the first match if found, or the
//               empty string if not found.
////////////////////////////////////////////////////////////////////
Filename DSearchPath::
find_file(const Filename &filename) const {
  if (filename.is_local()) {
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

  return string();
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::find_all_files
//       Access: Public
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

  return num_added;
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void DSearchPath::
output(ostream &out, const string &separator) const {
  if (!_directories.empty()) {
    Directories::const_iterator di = _directories.begin();
    out << (*di);
    ++di;
    while (di != _directories.end()) {
      out << separator << (*di);
      ++di;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::write
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void DSearchPath::
write(ostream &out, int indent_level) const {
  Directories::const_iterator di;
  for (di = _directories.begin(); di != _directories.end(); ++di) {
    for (int i = 0; i < indent_level; i++) {
      out << ' ';
    }
    out << (*di) << "\n";
  }
}


