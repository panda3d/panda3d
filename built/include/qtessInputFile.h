/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file qtessInputFile.h
 * @author drose
 * @date 2003-10-13
 */

#ifndef QTESSINPUTFILE_H
#define QTESSINPUTFILE_H

#include "pandatoolbase.h"
#include "qtessInputEntry.h"
#include "filename.h"
#include "pvector.h"
#include "vector_double.h"

class QtessSurface;

/**
 * Stores all the information read from a tesselation input file: a list of
 * QtessInputEntry's.
 */
class QtessInputFile {
public:
  QtessInputFile();
  INLINE QtessInputFile(const QtessInputFile &copy);
  INLINE void operator = (const QtessInputFile &copy);

  bool read(const Filename &filename);
  QtessInputEntry &get_default_entry();

  QtessInputEntry::Type match(QtessSurface *surface);
  int count_tris();

  void write(std::ostream &out, int indent_level = 0) const;

private:
  void add_default_entry();

  Filename _filename;

  typedef pvector<QtessInputEntry> Entries;
  Entries _entries;
};

#include "qtessInputFile.I"

#endif
