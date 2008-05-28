// Filename: cvsSourceTree.h
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

#ifndef CVSSOURCETREE_H
#define CVSSOURCETREE_H

#include "pandatoolbase.h"

#include "pvector.h"
#include "pmap.h"
#include "filename.h"

class CVSSourceDirectory;

////////////////////////////////////////////////////////////////////
//       Class : CVSSourceTree
// Description : This represents the root of the tree of source
//               directory files.
//
//               The tree is maintained in a case-insensitive manner,
//               even on a non-Windows system, since you might want to
//               eventually check out the CVS tree onto a Windows
//               system--and if you do, you'll be sad if there are
//               case conflicts within the tree.  So we make an effort
//               to ensure this doesn't happen by treating two files
//               with a different case as the same file.
////////////////////////////////////////////////////////////////////
class CVSSourceTree {
public:
  CVSSourceTree();
  ~CVSSourceTree();

  void set_root(const Filename &root_path);
  bool scan(const Filename &key_filename);

  CVSSourceDirectory *get_root() const;
  CVSSourceDirectory *find_directory(const Filename &path);
  CVSSourceDirectory *find_relpath(const string &relpath);
  CVSSourceDirectory *find_dirname(const string &dirname);

  // This nested class represents the selection of a particular
  // directory in which to place a given file, given its basename.
  // The basename of the file is returned as part of the answer,
  // because it might have changed in case from the original basename
  // (in order to match the case of an existing file in the selected
  // directory).
  class FilePath {
  public:
    FilePath();
    FilePath(CVSSourceDirectory *dir, const string &basename);
    bool is_valid() const;
    Filename get_path() const;
    Filename get_fullpath() const;
    Filename get_rel_from(const CVSSourceDirectory *other) const;

    CVSSourceDirectory *_dir;
    string _basename;
  };

  FilePath choose_directory(const string &basename,
                            CVSSourceDirectory *suggested_dir,
                            bool force, bool interactive);

  Filename get_root_fullpath();
  Filename get_root_dirname() const;

  static bool temp_chdir(const Filename &path);
  static void restore_cwd();

public:
  void add_file(const string &basename, CVSSourceDirectory *dir);

private:
  typedef pvector<FilePath> FilePaths;

  FilePath
  prompt_user(const string &basename, CVSSourceDirectory *suggested_dir,
              const FilePaths &paths, bool force, bool interactive);

  FilePath ask_existing(const string &filename, const FilePath &path);
  FilePath ask_existing(const string &filename, const FilePaths &paths,
                        CVSSourceDirectory *suggested_dir);
  FilePath ask_new(const string &filename, CVSSourceDirectory *dir);
  FilePath ask_any(const string &filename, const FilePaths &paths);

  string prompt(const string &message);

  static Filename get_actual_fullpath(const Filename &path);
  static Filename get_start_fullpath();

private:
  Filename _path;
  CVSSourceDirectory *_root;

  typedef pmap<string, FilePaths> Basenames;
  Basenames _basenames;

  static bool _got_start_fullpath;
  static Filename _start_fullpath;
  bool _got_root_fullpath;
  Filename _root_fullpath;
};

#endif
