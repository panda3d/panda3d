// Filename: qtessInputFile.h
// Created by:  drose (13Oct03)
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

#ifndef QTESSINPUTFILE_H
#define QTESSINPUTFILE_H

#include "pandatoolbase.h"
#include "qtessInputEntry.h"
#include "filename.h"
#include "pvector.h"

class QtessSurface;

////////////////////////////////////////////////////////////////////
//       Class : QtessInputFile
// Description : Stores all the information read from a tesselation
//               input file: a list of QtessInputEntry's.
////////////////////////////////////////////////////////////////////
class QtessInputFile {
public:
  QtessInputFile();
  INLINE QtessInputFile(const QtessInputFile &copy);
  INLINE void operator = (const QtessInputFile &copy);

  bool read(const Filename &filename);
  QtessInputEntry &get_default_entry();

  QtessInputEntry::Type match(QtessSurface *surface);
  int count_tris();

  void write(ostream &out, int indent_level = 0) const;

private:
  void add_default_entry();

  Filename _filename;

  typedef pvector<QtessInputEntry> Entries;
  Entries _entries;
};

#include "qtessInputFile.I"

#endif
