// Filename: memoryUsagePointerCounts.h
// Created by:  drose (04Jun01)
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

#ifndef MEMORYUSAGEPOINTERCOUNTS_H
#define MEMORYUSAGEPOINTERCOUNTS_H

#include <pandabase.h>

class MemoryInfo;

#ifndef NDEBUG
////////////////////////////////////////////////////////////////////
//       Class : MemoryUsagePointerCounts
// Description : This is a supporting class for MemoryUsage.  It
//               tracks the relative counts of a number of pointers of
//               some type (or age), for use by TypeHistogram and
//               AgeHistogram.  It's not exported from the DLL, and it
//               doesn't even exist if we're compiling NDEBUG.
////////////////////////////////////////////////////////////////////
class MemoryUsagePointerCounts {
public:
  INLINE MemoryUsagePointerCounts();
  INLINE MemoryUsagePointerCounts(const MemoryUsagePointerCounts &copy);
  INLINE void operator = (const MemoryUsagePointerCounts &copy);

  INLINE void clear();
  void add_info(MemoryInfo &info);
  void output(ostream &out) const;

  INLINE bool is_size_unknown() const;
  INLINE size_t get_size() const;
  INLINE int get_count() const;

  INLINE bool operator < (const MemoryUsagePointerCounts &other) const;

private:
  static void output_bytes(ostream &out, size_t size);

private:
  int _count;
  int _unknown_size_count;
  size_t _size;
};

INLINE ostream &operator << (ostream &out, const MemoryUsagePointerCounts &c);

#include "memoryUsagePointerCounts.I"

#endif  // NDEBUG

#endif

