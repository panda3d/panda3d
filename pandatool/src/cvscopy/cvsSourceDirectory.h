// Filename: cvsSourceDirectory.h
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
////////////////////////////////////////////////////////////////////
class CVSSourceDirectory {
public:
  CVSSourceDirectory(CVSSourceTree *tree, CVSSourceDirectory *parent,
                     const string &dirname);
  ~CVSSourceDirectory();

  string get_dirname() const;
  string get_fullpath() const;
  string get_path() const;
  string get_rel_to(const CVSSourceDirectory *other) const;

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
