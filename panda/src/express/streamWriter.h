// Filename: streamWriter.h
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

#ifndef STREAMWRITER_H
#define STREAMWRITER_H

#include "pandabase.h"

#include "numeric_types.h"
#include "littleEndian.h"
#include "bigEndian.h"

////////////////////////////////////////////////////////////////////
//       Class : StreamWriter
// Description : A StreamWriter object is used to write sequential
//               binary data directly to an ostream.  Its interface is
//               very similar to Datagram by design; it's primarily
//               intended as a convenience to eliminate the overhead
//               of writing bytes to a Datagram and then writing the
//               Datagram to a stream.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS StreamWriter {
public:
  INLINE StreamWriter(ostream &out);
PUBLISHED:
  INLINE StreamWriter(ostream *out);
  INLINE StreamWriter(const StreamWriter &copy);
  INLINE void operator = (const StreamWriter &copy);
  INLINE ~StreamWriter();

  INLINE ostream *get_ostream() const;

  INLINE void add_bool(bool value);
  INLINE void add_int8(PN_int8 value);
  INLINE void add_uint8(PN_uint8 value);

  // The default numeric packing is little-endian.
  INLINE void add_int16(PN_int16 value);
  INLINE void add_int32(PN_int32 value);
  INLINE void add_int64(PN_int64 value);
  INLINE void add_uint16(PN_uint16 value);
  INLINE void add_uint32(PN_uint32 value);
  INLINE void add_uint64(PN_uint64 value);
  INLINE void add_float32(float value);
  INLINE void add_float64(PN_float64 value);

  // These functions pack numbers big-endian, in case that's desired.
  INLINE void add_be_int16(PN_int16 value);
  INLINE void add_be_int32(PN_int32 value);
  INLINE void add_be_int64(PN_int64 value);
  INLINE void add_be_uint16(PN_uint16 value);
  INLINE void add_be_uint32(PN_uint32 value);
  INLINE void add_be_uint64(PN_uint64 value);
  INLINE void add_be_float32(float value);
  INLINE void add_be_float64(PN_float64 value);

  INLINE void add_string(const string &str);
  INLINE void add_string32(const string &str);
  INLINE void add_z_string(string str);
  INLINE void add_fixed_string(const string &str, size_t size);

  void pad_bytes(size_t size);
  INLINE void append_data(const void *data, size_t size);
  INLINE void append_data(const string &data);

private:
  ostream *_out;
};

#include "streamWriter.I"

#endif
