// Filename: dSearchPath.h
// Created by:  drose (01Jul00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef PANDASEARCHPATH_H
#define PANDASEARCHPATH_H

#include "ppremake.h"

#include "filename.h"
#include <vector>

#define pvector std::vector

///////////////////////////////////////////////////////////////////
//       Class : DSearchPath
// Description : This class stores a list of directories that can be
//               searched, in order, to locate a particular file.  It
//               is normally constructed by passing it a traditional
//               searchpath-style string, e.g. a list of directory
//               names delimited by spaces or colons, but it can also
//               be built up explicitly.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOL DSearchPath {
public:
  class EXPCL_DTOOL Results {
  PUBLISHED:
    Results();
    Results(const Results &copy);
    void operator = (const Results &copy);
    ~Results();

    void clear();
    int get_num_files() const;
    const Filename &get_file(int n) const;

  public:
    void add_file(const Filename &file);

  private:
    typedef pvector<Filename> Files;
    Files _files;
  };

PUBLISHED:
  DSearchPath();
  DSearchPath(const string &path, const string &delimiters = ": \n\t");
  DSearchPath(const DSearchPath &copy);
  void operator = (const DSearchPath &copy);
  ~DSearchPath();

  void clear();
  void append_directory(const Filename &directory);
  void prepend_directory(const Filename &directory);
  void append_path(const string &path,
                   const string &delimiters = ": \n\t");
  void append_path(const DSearchPath &path);
  void prepend_path(const DSearchPath &path);

  bool is_empty() const;
  int get_num_directories() const;
  const Filename &get_directory(int n) const;

  Filename find_file(const Filename &filename) const;
  int find_all_files(const Filename &filename, Results &results) const;

  INLINE static Filename
  search_path(const Filename &filename, const string &path,
              const string &delimiters = ": \n\t");

  void output(ostream &out, const string &separator = ":") const;
  void write(ostream &out, int indent_level = 0) const;

private:
  typedef pvector<Filename> Directories;
  Directories _directories;
};

INLINE ostream &operator << (ostream &out, const DSearchPath &sp) {
  sp.output(out);
  return out;
}

#include "dSearchPath.I"

#endif
