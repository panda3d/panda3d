// Filename: nativeNumericData.h
// Created by:  drose (09May01)
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

#ifndef NATIVENUMERICDATA_H
#define NATIVENUMERICDATA_H

#include <pandabase.h>

#include <string.h>  // for memcpy()

////////////////////////////////////////////////////////////////////
//       Class : NativeNumericData
// Description : NativeNumericData and ReversedNumericData work
//               together to provide a sneaky interface for
//               automatically byte-swapping numbers, when necessary,
//               to transparency support big-endian and little-endian
//               architectures.
//
//               Both of these classes provide interfaces that accept
//               a pointer to a numeric variable and the size of the
//               number, and they can append that data to the end of a
//               string, or memcpy it into another location.
//
//               The difference is that NativeNumericData simply
//               passes everything through unchanged, while
//               ReversedNumericData always byte-swaps everything.
//               Otherwise, they have the same interface.
//
//               The transparent part comes from LittleEndian and
//               BigEndian, which are typedeffed to be one of these or
//               the other, according to the machine's architecture.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS NativeNumericData {
public:
  INLINE NativeNumericData(const void *data, size_t length);
  INLINE NativeNumericData(const string &data, size_t start, size_t length);

  INLINE void store_value(void *dest, size_t length);
  INLINE void append_to(string &dest, size_t length);

private:
  const void *_source;
};

#include "nativeNumericData.I"

#endif
