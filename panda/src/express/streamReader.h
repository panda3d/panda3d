// Filename: streamReader.h
// Created by:  drose (04Aug02)
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

#ifndef STREAMREADER_H
#define STREAMREADER_H

#include "pandabase.h"

#include "numeric_types.h"
#include "littleEndian.h"
#include "bigEndian.h"

////////////////////////////////////////////////////////////////////
//       Class : StreamReader
// Description : A class to read sequential binary data directly from
//               an istream.  Its interface is similar to
//               DatagramIterator by design; see also StreamWriter.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS StreamReader {
public:
  INLINE StreamReader(istream &in);
PUBLISHED:
  INLINE StreamReader(istream *in, bool owns_stream);
  INLINE StreamReader(const StreamReader &copy);
  INLINE void operator = (const StreamReader &copy);
  INLINE ~StreamReader();

  INLINE istream *get_istream() const;

  INLINE bool get_bool();
  INLINE PN_int8 get_int8();
  INLINE PN_uint8 get_uint8();

  INLINE PN_int16 get_int16();
  INLINE PN_int32 get_int32();
  INLINE PN_int64 get_int64();
  INLINE PN_uint16 get_uint16();
  INLINE PN_uint32 get_uint32();
  INLINE PN_uint64 get_uint64();
  INLINE float get_float32();
  INLINE PN_float64 get_float64();

  INLINE PN_int16 get_be_int16();
  INLINE PN_int32 get_be_int32();
  INLINE PN_int64 get_be_int64();
  INLINE PN_uint16 get_be_uint16();
  INLINE PN_uint32 get_be_uint32();
  INLINE PN_uint64 get_be_uint64();
  INLINE float get_be_float32();
  INLINE PN_float64 get_be_float64();

  string get_string();
  string get_string32();
  string get_z_string();
  string get_fixed_string(size_t size);

  void skip_bytes(size_t size);
  string extract_bytes(size_t size);

  string readline();

private:
  istream *_in;
  bool _owns_stream;
};

#include "streamReader.I"

#endif
