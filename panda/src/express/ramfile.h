// Filename: ramfile.h
// Created by:  mike (09Jan97)
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

  INLINE const string &get_data() const;

public:
  size_t _pos;
  string _data;
};

#include "ramfile.I"

#endif
