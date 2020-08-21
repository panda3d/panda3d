/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dSearchPath.cxx
 * @author drose
 * @date 2000-07-01
 */

#include "dSearchPath.h"
#include "filename.h"

#include <algorithm>
#include <iterator>

using std::ostream;
using std::string;

/**
 *
 */
DSearchPath::Results::
Results() {
}

/**
 *
 */
DSearchPath::Results::
Results(const DSearchPath::Results &copy) :
  _files(copy._files)
{
}

/**
 *
 */
void DSearchPath::Results::
operator = (const DSearchPath::Results &copy) {
  _files = copy._files;
}

/**
 *
 */
DSearchPath::Results::
~Results() {
}

/**
 * Removes all the files from the list.
 */
void DSearchPath::Results::
clear() {
  _files.clear();
}

/**
 * Returns the number of files on the result list.
 */
size_t DSearchPath::Results::
get_num_files() const {
  return _files.size();
}

/**
 * Returns the nth file on the result list.
 */
const Filename &DSearchPath::Results::
get_file(size_t n) const {
  assert(n < _files.size());
  return _files[n];
}

/**
 * Adds a new file to the result list.
 */
void DSearchPath::Results::
add_file(const Filename &file) {
  _files.push_back(file);
}

/**
 *
 */
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

/**
 *
 */
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

/**
 *
 */
DSearchPath::
DSearchPath(const string &path, const string &separator) {
  append_path(path, separator);
}

/**
 *
 */
DSearchPath::
DSearchPath(const Filename &directory) {
  append_directory(directory);
}

/**
 * Removes all the directories from the search list.
 */
void DSearchPath::
clear() {
  _directories.clear();
}

/**
 * Adds a new directory to the end of the search list.
 */
void DSearchPath::
append_directory(const Filename &directory) {
  _directories.push_back(directory);
}

/**
 * Adds a new directory to the front of the search list.
 */
void DSearchPath::
prepend_directory(const Filename &directory) {
  _directories.insert(_directories.begin(), directory);
}

/**
 * Adds all of the directories listed in the search path to the end of the
 * search list.
 */
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

/**
 * Adds all of the directories listed in the search path to the end of the
 * search list.
 */
void DSearchPath::
append_path(const DSearchPath &path) {
  std::copy(path._directories.begin(), path._directories.end(),
            std::back_inserter(_directories));
}

/**
 * Adds all of the directories listed in the search path to the beginning of
 * the search list.
 */
void DSearchPath::
prepend_path(const DSearchPath &path) {
  if (!path._directories.empty()) {
    Directories new_directories = path._directories;
    std::copy(_directories.begin(), _directories.end(),
              std::back_inserter(new_directories));
    _directories.swap(new_directories);
  }
}

/**
 * Returns true if the search list is empty, false otherwise.
 */
bool DSearchPath::
is_empty() const {
  return _directories.empty();
}

/**
 * Returns the number of directories on the search list.
 */
size_t DSearchPath::
get_num_directories() const {
  return _directories.size();
}

/**
 * Returns the nth directory on the search list.
 */
const Filename &DSearchPath::
get_directory(size_t n) const {
  assert(n < _directories.size());
  return _directories[n];
}

/**
 * Searches all the directories in the search list for the indicated file, in
 * order.  Returns the full matching pathname of the first match if found, or
 * the empty string if not found.
 */
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
            // A special case for the "." directory: to avoid prefixing an
            // endless stream of . in front of files, if the filename already
            // has a . prefixed (i.e.  is_fully_qualified() is true), we don't
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

/**
 * Searches all the directories in the search list for the indicated file, in
 * order.  Fills up the results list with *all* of the matching filenames
 * found, if any.  Returns the number of matches found.
 *
 * It is the responsibility of the the caller to clear the results list first;
 * otherwise, the newly-found files will be appended to the list.
 */
size_t DSearchPath::
find_all_files(const Filename &filename,
               DSearchPath::Results &results) const {
  size_t num_added = 0;

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
            // A special case for the "." directory: to avoid prefixing an
            // endless stream of . in front of files, if the filename already
            // has a . prefixed (i.e.  is_fully_qualified() is true), we don't
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

/**
 *
 */
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

/**
 *
 */
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
