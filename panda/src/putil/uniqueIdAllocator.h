// Filename: uniqueIdAllocator.h
// Created by:  schuyler 2003-03-13
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
// 

#ifndef _UNIQUEIDALLOCATOR_H //[
#define _UNIQUEIDALLOCATOR_H

#include "pandabase.h"

typedef unsigned long U32;


////////////////////////////////////////////////////////////////////
//       Class : UniqueIdAllocator
// Description : Manage a set of ID values from min to max inclusive.
//               The ID numbers that are freed will be allocated
//               (reused) in the same order.  I.e. the oldest ID numbers
//               will be allocated.
//
//               This implementation will use 4 bytes per id number,
//               plus a few bytes of management data.  e.g. 10,000
//               ID numbers will use 40KB.
//
//               There are other implementations that can better leverage
//               runs of used or unused IDs or use bit arrays for the
//               IDs.  But, it takes extra work to track the age of
//               freed IDs, which is required for what we wanted.  If
//               you would like to kick around other implementation
//               ideas, please contact Schuyler.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA UniqueIdAllocator {
PUBLISHED:
  UniqueIdAllocator(U32 min=0, U32 max=20);
  ~UniqueIdAllocator();
  U32 allocate();
  void free(U32 index);
  float percent_used() const;
  void output(ostream& os, bool verbose=false) const;

protected:
  U32* _table;
  U32 _min;
  U32 _max;
  U32 _next_free;
  U32 _last_free;
  U32 _size;
  U32 _free;
};

#endif //]
