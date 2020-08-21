/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file streamReader.h
 * @author drose
 * @date 2002-08-04
 */

#ifndef STREAMREADER_H
#define STREAMREADER_H

#include "dtoolbase.h"
#include "pnotify.h"
#include "numeric_types.h"
#include "littleEndian.h"
#include "bigEndian.h"
#include "vector_uchar.h"

/**
 * A class to read sequential binary data directly from an istream.  Its
 * interface is similar to DatagramIterator by design; see also StreamWriter.
 */
class EXPCL_DTOOL_PRC StreamReader {
public:
  INLINE StreamReader(std::istream &in);
PUBLISHED:
  INLINE explicit StreamReader(std::istream *in, bool owns_stream);
  INLINE StreamReader(const StreamReader &copy);
  INLINE StreamReader(StreamReader &&from) noexcept;
  INLINE void operator = (const StreamReader &copy);
  INLINE void operator = (StreamReader &&from) noexcept;
  INLINE ~StreamReader();

  INLINE std::istream *get_istream() const;
  MAKE_PROPERTY(std::istream, get_istream);

  BLOCKING INLINE bool get_bool();
  BLOCKING INLINE int8_t get_int8();
  BLOCKING INLINE uint8_t get_uint8();

  BLOCKING INLINE int16_t get_int16();
  BLOCKING INLINE int32_t get_int32();
  BLOCKING INLINE int64_t get_int64();
  BLOCKING INLINE uint16_t get_uint16();
  BLOCKING INLINE uint32_t get_uint32();
  BLOCKING INLINE uint64_t get_uint64();
  BLOCKING INLINE float get_float32();
  BLOCKING INLINE PN_float64 get_float64();

  BLOCKING INLINE int16_t get_be_int16();
  BLOCKING INLINE int32_t get_be_int32();
  BLOCKING INLINE int64_t get_be_int64();
  BLOCKING INLINE uint16_t get_be_uint16();
  BLOCKING INLINE uint32_t get_be_uint32();
  BLOCKING INLINE uint64_t get_be_uint64();
  BLOCKING INLINE float get_be_float32();
  BLOCKING INLINE PN_float64 get_be_float64();

  BLOCKING std::string get_string();
  BLOCKING std::string get_string32();
  BLOCKING std::string get_z_string();
  BLOCKING std::string get_fixed_string(size_t size);

  BLOCKING void skip_bytes(size_t size);
  BLOCKING size_t extract_bytes(unsigned char *into, size_t size);
  EXTENSION(BLOCKING PyObject *extract_bytes(size_t size));

  EXTENSION(BLOCKING PyObject *readline());
  EXTENSION(BLOCKING PyObject *readlines());

public:
  BLOCKING vector_uchar extract_bytes(size_t size);
  BLOCKING std::string readline();

private:
  std::istream *_in;
  bool _owns_stream;
};

#include "streamReader.I"

#endif
