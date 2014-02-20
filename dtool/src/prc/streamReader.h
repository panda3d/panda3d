// Filename: streamReader.h
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

#ifndef STREAMREADER_H
#define STREAMREADER_H

#include "dtoolbase.h"
#include "pnotify.h"
#include "numeric_types.h"
#include "littleEndian.h"
#include "bigEndian.h"

////////////////////////////////////////////////////////////////////
//       Class : StreamReader
// Description : A class to read sequential binary data directly from
//               an istream.  Its interface is similar to
//               DatagramIterator by design; see also StreamWriter.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOLCONFIG StreamReader {
public:
  INLINE StreamReader(istream &in);
PUBLISHED:
  INLINE StreamReader(istream *in, bool owns_stream);
  INLINE StreamReader(const StreamReader &copy);
  INLINE void operator = (const StreamReader &copy);
  INLINE ~StreamReader();

  INLINE istream *get_istream() const;

  BLOCKING INLINE bool get_bool();
  BLOCKING INLINE PN_int8 get_int8();
  BLOCKING INLINE PN_uint8 get_uint8();

  BLOCKING INLINE PN_int16 get_int16();
  BLOCKING INLINE PN_int32 get_int32();
  BLOCKING INLINE PN_int64 get_int64();
  BLOCKING INLINE PN_uint16 get_uint16();
  BLOCKING INLINE PN_uint32 get_uint32();
  BLOCKING INLINE PN_uint64 get_uint64();
  BLOCKING INLINE float get_float32();
  BLOCKING INLINE PN_float64 get_float64();

  BLOCKING INLINE PN_int16 get_be_int16();
  BLOCKING INLINE PN_int32 get_be_int32();
  BLOCKING INLINE PN_int64 get_be_int64();
  BLOCKING INLINE PN_uint16 get_be_uint16();
  BLOCKING INLINE PN_uint32 get_be_uint32();
  BLOCKING INLINE PN_uint64 get_be_uint64();
  BLOCKING INLINE float get_be_float32();
  BLOCKING INLINE PN_float64 get_be_float64();

  BLOCKING string get_string();
  BLOCKING string get_string32();
  BLOCKING string get_z_string();
  BLOCKING string get_fixed_string(size_t size);

  BLOCKING void skip_bytes(size_t size);
  BLOCKING string extract_bytes(size_t size);
  BLOCKING size_t extract_bytes(unsigned char *into, size_t size);

  BLOCKING string readline();
  EXTENSION(BLOCKING PyObject *readlines());

private:
  istream *_in;
  bool _owns_stream;
};

#include "streamReader.I"

#endif
