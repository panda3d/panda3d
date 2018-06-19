/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppFile.h
 * @author drose
 * @date 1999-11-11
 */

#ifndef CPPFILE_H
#define CPPFILE_H

#include "dtoolbase.h"
#include "filename.h"

/**
 * This defines a source file (typically a C++ header file) that is parsed by
 * the CPPParser.  Each declaration indicates the source file where it
 * appeared.
 */
class CPPFile {
public:
  enum Source {
    S_local,       // File resides in the current directory
    S_alternate,   // File resides in some other directory
    S_system,      // File resides in a system directory
    S_none,        // File is internally generated
  };

  CPPFile(const Filename &filename = "",
          const Filename &filename_as_referenced = "",
          Source source = S_none);
  CPPFile(const CPPFile &copy);
  void operator = (const CPPFile &copy);
  ~CPPFile();

  bool is_c_or_i_file() const;
  static bool is_c_or_i_file(const Filename &filename);

  bool is_c_file() const;
  static bool is_c_file(const Filename &filename);

  void replace_nearer(const CPPFile &other);

  bool operator < (const CPPFile &other) const;
  bool operator == (const CPPFile &other) const;
  bool operator != (const CPPFile &other) const;

  const char *c_str() const;
  bool empty() const;

  Filename _filename;
  Filename _filename_as_referenced;
  mutable Source _source;
  mutable bool _pragma_once;
};

inline std::ostream &operator << (std::ostream &out, const CPPFile &file) {
  return out << file._filename;
}

#endif
