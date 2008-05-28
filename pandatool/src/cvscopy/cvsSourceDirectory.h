// Filename: cvsSourceDirectory.h
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

#ifndef CVSSOURCEDIRECTORY_H
#define CVSSOURCEDIRECTORY_H

#include "pandatoolbase.h"
#include "filename.h"

#include "pvector.h"

class CVSSourceTree;

////////////////////////////////////////////////////////////////////
//       Class : CVSSourceDirectory
// Description : This represents one particular directory in the
//               hierarchy of source directory files.  We must scan
//               the source directory to identify where the related
//               files have previously been copied.
//
//               The tree is maintained in a case-insensitive manner,
//               even on a non-Windows system, since you might want to
//               eventually check out the CVS tree onto a Windows
//               system--and if you do, you'll be sad if there are
//               case conflicts within the tree.  So we make an effort
//               to ensure this doesn't happen by treating two files
//               with a different case as the same file.
////////////////////////////////////////////////////////////////////
class CVSSourceDirectory {
public:
  CVSSourceDirectory(CVSSourceTree *tree, CVSSourceDirectory *parent,
                     const string &dirname);
  ~CVSSourceDirectory();

  string get_dirname() const;
  Filename get_fullpath() const;
  Filename get_path() const;
  Filename get_rel_to(const CVSSourceDirectory *other) const;

  int get_num_children() const;
  CVSSourceDirectory *get_child(int n) const;

  CVSSourceDirectory *find_relpath(const string &relpath);
  CVSSourceDirectory *find_dirname(const string &dirname);

public:
  bool scan(const Filename &directory, const string &key_filename);

private:
  CVSSourceTree *_tree;
  CVSSourceDirectory *_parent;
  string _dirname;
  int _depth;

  typedef pvector<CVSSourceDirectory *> Children;
  Children _children;
};

#endif
