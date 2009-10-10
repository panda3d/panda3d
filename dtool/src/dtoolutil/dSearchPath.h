// Filename: dSearchPath.h
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

#ifndef PANDASEARCHPATH_H
#define PANDASEARCHPATH_H

#include "dtoolbase.h"

#include "filename.h"
#include "pvector.h"

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
PUBLISHED:
  class EXPCL_DTOOL Results {
  PUBLISHED:
    Results();
    Results(const Results &copy);
    void operator = (const Results &copy);
    ~Results();

    void clear();
    int get_num_files() const;
    const Filename &get_file(int n) const;

    INLINE Filename operator [] (int n) const;
    INLINE int size() const;

    void output(ostream &out) const;
    void write(ostream &out, int indent_level = 0) const;

  public:
    void add_file(const Filename &file);

  private:
    typedef pvector<Filename> Files;
    Files _files;
  };

  DSearchPath();
  DSearchPath(const string &path, const string &separator = string());
  DSearchPath(const Filename &directory);
  DSearchPath(const DSearchPath &copy);
  void operator = (const DSearchPath &copy);
  ~DSearchPath();

  void clear();
  void append_directory(const Filename &directory);
  void prepend_directory(const Filename &directory);
  void append_path(const string &path,
                   const string &separator = string());
  void append_path(const DSearchPath &path);
  void prepend_path(const DSearchPath &path);

  bool is_empty() const;
  int get_num_directories() const;
  const Filename &get_directory(int n) const;
  MAKE_SEQ(get_directories, get_num_directories, get_directory);

  Filename find_file(const Filename &filename) const;
  int find_all_files(const Filename &filename, Results &results) const;
  INLINE Results find_all_files(const Filename &filename) const;

  INLINE static Filename
  search_path(const Filename &filename, const string &path,
              const string &separator = string());

  void output(ostream &out, const string &separator = string()) const;
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
