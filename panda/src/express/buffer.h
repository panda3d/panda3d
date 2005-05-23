// Filename: buffer.h
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
  INLINE int get_length(void) const;

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
