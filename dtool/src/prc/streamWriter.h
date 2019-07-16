/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file streamWriter.h
 * @author drose
 * @date 2002-08-04
 */

#ifndef STREAMWRITER_H
#define STREAMWRITER_H

#include "dtoolbase.h"
#include "pnotify.h"
#include "numeric_types.h"
#include "littleEndian.h"
#include "bigEndian.h"

/**
 * A StreamWriter object is used to write sequential binary data directly to
 * an ostream.  Its interface is very similar to Datagram by design; it's
 * primarily intended as a convenience to eliminate the overhead of writing
 * bytes to a Datagram and then writing the Datagram to a stream.
 */
class EXPCL_DTOOL_PRC StreamWriter {
public:
  INLINE StreamWriter(std::ostream &out);
PUBLISHED:
  INLINE explicit StreamWriter(std::ostream *out, bool owns_stream);
  INLINE StreamWriter(const StreamWriter &copy);
  INLINE StreamWriter(StreamWriter &&from) noexcept;
  INLINE void operator = (const StreamWriter &copy);
  INLINE void operator = (StreamWriter &&from) noexcept;
  INLINE ~StreamWriter();

  INLINE std::ostream *get_ostream() const;
  MAKE_PROPERTY(std::ostream, get_ostream);

  BLOCKING INLINE void add_bool(bool value);
  BLOCKING INLINE void add_int8(int8_t value);
  BLOCKING INLINE void add_uint8(uint8_t value);

  // The default numeric packing is little-endian.
  BLOCKING INLINE void add_int16(int16_t value);
  BLOCKING INLINE void add_int32(int32_t value);
  BLOCKING INLINE void add_int64(int64_t value);
  BLOCKING INLINE void add_uint16(uint16_t value);
  BLOCKING INLINE void add_uint32(uint32_t value);
  BLOCKING INLINE void add_uint64(uint64_t value);
  BLOCKING INLINE void add_float32(float value);
  BLOCKING INLINE void add_float64(PN_float64 value);

  // These functions pack numbers big-endian, in case that's desired.
  BLOCKING INLINE void add_be_int16(int16_t value);
  BLOCKING INLINE void add_be_int32(int32_t value);
  BLOCKING INLINE void add_be_int64(int64_t value);
  BLOCKING INLINE void add_be_uint16(uint16_t value);
  BLOCKING INLINE void add_be_uint32(uint32_t value);
  BLOCKING INLINE void add_be_uint64(uint64_t value);
  BLOCKING INLINE void add_be_float32(float value);
  BLOCKING INLINE void add_be_float64(PN_float64 value);

  BLOCKING INLINE void add_string(const std::string &str);
  BLOCKING INLINE void add_string32(const std::string &str);
  BLOCKING INLINE void add_z_string(std::string str);
  BLOCKING INLINE void add_fixed_string(const std::string &str, size_t size);

  BLOCKING void pad_bytes(size_t size);
  EXTENSION(void append_data(PyObject *data));

  BLOCKING INLINE void flush();

  BLOCKING INLINE void write(const std::string &str);

public:
  BLOCKING INLINE void append_data(const void *data, size_t size);
  BLOCKING INLINE void append_data(const std::string &data);

private:
  std::ostream *_out;
  bool _owns_stream;

#ifdef HAVE_PYTHON
PUBLISHED:
  // Python 2 needs this for printing to work correctly.
  int softspace;
#endif
};

#include "streamWriter.I"

#endif
