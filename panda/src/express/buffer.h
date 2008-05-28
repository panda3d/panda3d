// Filename: buffer.h
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

#ifndef BUFFER_H
#define BUFFER_H

#include "pandabase.h"
#include "typedef.h"
#include "referenceCount.h"

////////////////////////////////////////////////////////////////////
//       Class : Buffer
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS Buffer : public ReferenceCount {
public:
  Buffer(int size);
  ~Buffer();

PUBLISHED:
  INLINE int get_length() const;

#ifndef CPPPARSER
// hidden from interrogate
public:
  char *_buffer;
#endif

private:
  int _length;
};

#include "buffer.I"

#endif
