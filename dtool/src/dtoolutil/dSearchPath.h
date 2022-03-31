/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dSearchPath.h
 * @author drose
 * @date 2000-07-01
 */

#ifndef PANDASEARCHPATH_H
#define PANDASEARCHPATH_H

#include "dtoolbase.h"

#include "filename.h"
#include "pvector.h"

/**
 * This class stores a list of directories that can be searched, in order, to
 * locate a particular file.  It is normally constructed by passing it a
 * traditional searchpath-style string, e.g.  a list of directory names
 * delimited by spaces or colons, but it can also be built up explicitly.
 */
class EXPCL_DTOOL_DTOOLUTIL DSearchPath {
PUBLISHED:
  class EXPCL_DTOOL_DTOOLUTIL Results {
  PUBLISHED:
    Results();
    Results(const Results &copy);
    void operator = (const Results &copy);
    ~Results();

    void clear();
    size_t get_num_files() const;
    const Filename &get_file(size_t n) const;

    INLINE Filename operator [] (size_t n) const;
    INLINE size_t size() const;

    void output(std::ostream &out) const;
    void write(std::ostream &out, int indent_level = 0) const;

  public:
    void add_file(const Filename &file);

  private:
    typedef pvector<Filename> Files;
    Files _files;
  };

  DSearchPath() = default;
  DSearchPath(const std::string &path, const std::string &separator = std::string());
  DSearchPath(const Filename &directory);
  DSearchPath(const DSearchPath &copy) = default;
  DSearchPath(DSearchPath &&from) = default;
  ~DSearchPath() = default;

  DSearchPath &operator = (const DSearchPath &copy) = default;
  DSearchPath &operator = (DSearchPath &&from) = default;

  void clear();
  void append_directory(const Filename &directory);
  void prepend_directory(const Filename &directory);
  void append_path(const std::string &path,
                   const std::string &separator = std::string());
  void append_path(const DSearchPath &path);
  void prepend_path(const DSearchPath &path);

  bool is_empty() const;
  size_t get_num_directories() const;
  const Filename &get_directory(size_t n) const;
  MAKE_SEQ(get_directories, get_num_directories, get_directory);
  MAKE_SEQ_PROPERTY(directories, get_num_directories, get_directory);

  Filename find_file(const Filename &filename) const;
  size_t find_all_files(const Filename &filename, Results &results) const;
  INLINE Results find_all_files(const Filename &filename) const;

  INLINE static Filename
  search_path(const Filename &filename, const std::string &path,
              const std::string &separator = std::string());

  void output(std::ostream &out, const std::string &separator = std::string()) const;
  void write(std::ostream &out, int indent_level = 0) const;

private:
  typedef pvector<Filename> Directories;
  Directories _directories;
};

INLINE std::ostream &operator << (std::ostream &out, const DSearchPath &sp) {
  sp.output(out);
  return out;
}

#include "dSearchPath.I"

#endif
