// Filename: dcPackData.h
// Created by:  drose (15Jun04)
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

#ifndef DCPACKDATA_H
#define DCPACKDATA_H

#include "dcbase.h"

////////////////////////////////////////////////////////////////////
//       Class : DCPackData
// Description : This is a block of data that receives the results of
//               DCPacker.
////////////////////////////////////////////////////////////////////
class EXPCL_DIRECT DCPackData {
PUBLISHED:
  INLINE DCPackData();
  INLINE ~DCPackData();

  INLINE void clear();

public:
  INLINE void append_data(const char *buffer, size_t size);
  INLINE char *get_write_pointer(size_t size);
  INLINE void append_junk(size_t size);
  INLINE void rewrite_data(size_t position, const char *buffer, size_t size);
  INLINE char *get_rewrite_pointer(size_t position, size_t size);

PUBLISHED:
  INLINE string get_string() const;
  INLINE size_t get_length() const;
public:
  INLINE const char *get_data() const;
  INLINE char *take_data();

private:
  void set_used_length(size_t size);

private:
  char *_buffer;
  size_t _allocated_size;
  size_t _used_length;
};

#include "dcPackData.I"

#endif
