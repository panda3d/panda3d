// Filename: ramfile.h
// Created by:  mike (09Jan97)
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

#ifndef RAMFILE_H
#define RAMFILE_H

#include "pandabase.h"
#include "typedef.h"
#include "referenceCount.h"

////////////////////////////////////////////////////////////////////
//       Class : Ramfile
// Description : An in-memory buffer specifically designed for
//               downloading files to memory.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS Ramfile {
PUBLISHED:
  INLINE Ramfile();

  INLINE void seek(size_t pos);
  INLINE size_t tell() const;
  string read(size_t length);
  string readline();
  EXTENSION(PyObject *readlines());

  INLINE const string &get_data() const;
  INLINE size_t get_data_size() const;
  INLINE void clear();

public:
  size_t _pos;
  string _data;
};

#include "ramfile.I"

#endif
