// Filename: streamWriter.h
// Created by:  drose (04Aug02)
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

#ifndef STREAMWRITER_H
#define STREAMWRITER_H

#include "dtoolbase.h"
#include "pnotify.h"
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
class EXPCL_DTOOLCONFIG StreamWriter {
public:
  INLINE StreamWriter(ostream &out);
PUBLISHED:
  INLINE StreamWriter(ostream *out, bool owns_stream);
  INLINE StreamWriter(const StreamWriter &copy);
  INLINE void operator = (const StreamWriter &copy);
  INLINE ~StreamWriter();

  INLINE ostream *get_ostream() const;

  BLOCKING INLINE void add_bool(bool value);
  BLOCKING INLINE void add_int8(PN_int8 value);
  BLOCKING INLINE void add_uint8(PN_uint8 value);

  // The default numeric packing is little-endian.
  BLOCKING INLINE void add_int16(PN_int16 value);
  BLOCKING INLINE void add_int32(PN_int32 value);
  BLOCKING INLINE void add_int64(PN_int64 value);
  BLOCKING INLINE void add_uint16(PN_uint16 value);
  BLOCKING INLINE void add_uint32(PN_uint32 value);
  BLOCKING INLINE void add_uint64(PN_uint64 value);
  BLOCKING INLINE void add_float32(float value);
  BLOCKING INLINE void add_float64(PN_float64 value);

  // These functions pack numbers big-endian, in case that's desired.
  BLOCKING INLINE void add_be_int16(PN_int16 value);
  BLOCKING INLINE void add_be_int32(PN_int32 value);
  BLOCKING INLINE void add_be_int64(PN_int64 value);
  BLOCKING INLINE void add_be_uint16(PN_uint16 value);
  BLOCKING INLINE void add_be_uint32(PN_uint32 value);
  BLOCKING INLINE void add_be_uint64(PN_uint64 value);
  BLOCKING INLINE void add_be_float32(float value);
  BLOCKING INLINE void add_be_float64(PN_float64 value);

  BLOCKING INLINE void add_string(const string &str);
  BLOCKING INLINE void add_string32(const string &str);
  BLOCKING INLINE void add_z_string(string str);
  BLOCKING INLINE void add_fixed_string(const string &str, size_t size);

  BLOCKING void pad_bytes(size_t size);
  BLOCKING INLINE void append_data(const void *data, size_t size);
  BLOCKING INLINE void append_data(const string &data);

  BLOCKING INLINE void flush();

  BLOCKING INLINE void write(const string &str);

private:
  ostream *_out;
  bool _owns_stream;
};

#include "streamWriter.I"

#endif
