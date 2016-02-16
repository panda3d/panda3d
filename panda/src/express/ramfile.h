/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ramfile.h
 * @author mike
 * @date 1997-01-09
 */

#ifndef RAMFILE_H
#define RAMFILE_H

#include "pandabase.h"
#include "typedef.h"
#include "referenceCount.h"
#include "extension.h"

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
  EXTENSION(PyObject *read(size_t length));
  EXTENSION(PyObject *readline());
  EXTENSION(PyObject *readlines());

  EXTENSION(PyObject *get_data() const);
  INLINE size_t get_data_size() const;
  INLINE void clear();

public:
  string read(size_t length);
  string readline();
  INLINE const string &get_data() const;

  size_t _pos;
  string _data;

  friend class Extension<Ramfile>;
};

#include "ramfile.I"

#endif
