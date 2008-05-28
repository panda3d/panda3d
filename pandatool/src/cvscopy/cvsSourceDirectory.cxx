// Filename: cvsSourceDirectory.cxx
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

#include "cvsSourceDirectory.h"
#include "cvsSourceTree.h"
#include "string_utils.h"

#include "pnotify.h"

////////////////////////////////////////////////////////////////////
//     Function: CVSSourceDirectory::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CVSSourceDirectory::
CVSSourceDirectory(CVSSourceTree *tree, CVSSourceDirectory *parent,
                   const string &dirname) :
  _tree(tree),
  _parent(parent),
  _dirname(dirname)
{
  if (_parent == (CVSSourceDirectory *)NULL) {
    _depth = 0;
  } else {
    _depth = _parent->_depth + 1;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CVSSourceDirectory::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CVSSourceDirectory::
~CVSSourceDirectory() {
  Children::iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    delete (*ci);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CVSSourceDirectory::get_dirname
//       Access: Public
//  Description: Returns the local name of this particular directory.
////////////////////////////////////////////////////////////////////
string CVSSourceDirectory::
get_dirname() const {
  return _dirname;
}

////////////////////////////////////////////////////////////////////
//     Function: CVSSourceDirectory::get_fullpath
//       Access: Public
//  Description: Returns the full pathname to this particular
//               directory.
////////////////////////////////////////////////////////////////////
Filename CVSSourceDirectory::
get_fullpath() const {
  if (_parent == (CVSSourceDirectory *)NULL) {
    return _tree->get_root_fullpath();
  }
  return Filename(_parent->get_fullpath(), _dirname);
}

////////////////////////////////////////////////////////////////////
//     Function: CVSSourceDirectory::get_path
//       Access: Public
//  Description: Returns the relative pathname to this particular
//               directory, as seen from the root of the tree.
////////////////////////////////////////////////////////////////////
Filename CVSSourceDirectory::
get_path() const {
  if (_parent == (CVSSourceDirectory *)NULL) {
    return _dirname;
  }
  return Filename(_parent->get_path(), _dirname);
}

////////////////////////////////////////////////////////////////////
//     Function: CVSSourceDirectory::get_rel_to
//       Access: Public
//  Description: Returns the relative path to the other directory from
//               this one.  This does not include a trailing slash.
////////////////////////////////////////////////////////////////////
Filename CVSSourceDirectory::
get_rel_to(const CVSSourceDirectory *other) const {
  const CVSSourceDirectory *a = this;
  const CVSSourceDirectory *b = other;

  if (a == b) {
    return ".";
  }

  string prefix, postfix;
  while (a->_depth > b->_depth) {
    prefix += "../";
    a = a->_parent;
    nassertr(a != (CVSSourceDirectory *)NULL, string());
  }

  while (b->_depth > a->_depth) {
    postfix = b->_dirname + "/" + postfix;
    b = b->_parent;
    nassertr(b != (CVSSourceDirectory *)NULL, string());
  }

  while (a != b) {
    prefix += "../";
    postfix = b->_dirname + "/" + postfix;
    a = a->_parent;
    b = b->_parent;
    nassertr(a != (CVSSourceDirectory *)NULL, string());
    nassertr(b != (CVSSourceDirectory *)NULL, string());
  }

  string result = prefix + postfix;
  nassertr(!result.empty(), string());
  return result.substr(0, result.length() - 1);
}

////////////////////////////////////////////////////////////////////
//     Function: CVSSourceDirectory::get_num_children
//       Access: Public
//  Description: Returns the number of subdirectories below this
//               directory.
////////////////////////////////////////////////////////////////////
int CVSSourceDirectory::
get_num_children() const {
  return _children.size();
}

////////////////////////////////////////////////////////////////////
//     Function: CVSSourceDirectory::get_child
//       Access: Public
//  Description: Returns the nth subdirectory below this directory.
////////////////////////////////////////////////////////////////////
CVSSourceDirectory *CVSSourceDirectory::
get_child(int n) const {
  nassertr(n >= 0 && n < (int)_children.size(), NULL);
  return _children[n];
}

////////////////////////////////////////////////////////////////////
//     Function: CVSSourceDirectory::find_relpath
//       Access: Public
//  Description: Returns the source directory that corresponds to the
//               given relative path from this directory, or NULL if
//               there is no match.
////////////////////////////////////////////////////////////////////
CVSSourceDirectory *CVSSourceDirectory::
find_relpath(const string &relpath) {
  if (relpath.empty()) {
    return this;
  }

  size_t slash = relpath.find('/');
  string first = relpath.substr(0, slash);
  string rest;
  if (slash != string::npos) {
    rest = relpath.substr(slash + 1);
  }

  if (first.empty() || first == ".") {
    return find_relpath(rest);

  } else if (first == "..") {
    if (_parent != NULL) {
      return _parent->find_relpath(rest);
    }
    // Tried to back out past the root directory.
    return (CVSSourceDirectory *)NULL;
  }

  // Check for a child with the name indicated by first.
  Children::const_iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    if (cmp_nocase((*ci)->get_dirname(), first) == 0) {
      return (*ci)->find_relpath(rest);
    }
  }

  // No match.
  return (CVSSourceDirectory *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CVSSourceDirectory::find_dirname
//       Access: Public
//  Description: Returns the source directory that corresponds to the
//               given local directory name, or NULL if there is no
//               match.
////////////////////////////////////////////////////////////////////
CVSSourceDirectory *CVSSourceDirectory::
find_dirname(const string &dirname) {
  if (cmp_nocase(dirname, _dirname) == 0) {
    return this;
  }

  Children::const_iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    CVSSourceDirectory *result = (*ci)->find_dirname(dirname);
    if (result != (CVSSourceDirectory *)NULL) {
      return result;
    }
  }

  return (CVSSourceDirectory *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CVSSourceDirectory::scan
//       Access: Public
//  Description: Recursively scans the contents of the source
//               directory.  Fullpath is the full path name to the
//               directory; key_filename is the name of a file that
//               must exist in each subdirectory for it to be
//               considered part of the hierarchy.  Returns true on
//               success, false on failure.
////////////////////////////////////////////////////////////////////
bool CVSSourceDirectory::
scan(const Filename &directory, const string &key_filename) {
  vector_string contents;
  if (!directory.scan_directory(contents)) {
    nout << "Unable to scan directory " << directory << "\n";
    return false;
  }

  vector_string::const_iterator fi;
  for (fi = contents.begin(); fi != contents.end(); ++fi) {
    const string &basename = (*fi);

    // Is this possibly a subdirectory name?
    Filename next_path(directory, basename);
    Filename key(next_path, key_filename);

    if (key.exists()) {
      CVSSourceDirectory *subdir =
        new CVSSourceDirectory(_tree, this, basename);
      _children.push_back(subdir);

      if (!subdir->scan(next_path, key_filename)) {
        return false;
      }

    } else {
      // It's not a subdirectory; call it a regular file.
      _tree->add_file(basename, this);
    }
  }

  return true;
}
