/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file datagramIterator.h
 * @author jns
 * @date 2000-02-07
 */

#ifndef DATAGRAMITERATOR_H
#define DATAGRAMITERATOR_H

#include "pandabase.h"

#include "datagram.h"
#include "numeric_types.h"

/**
 * A class to retrieve the individual data elements previously stored in a
 * Datagram.  Elements may be retrieved one at a time; it is up to the caller
 * to know the correct type and order of each element.
 */
class EXPCL_PANDA_EXPRESS DatagramIterator {
public:
  INLINE void assign(Datagram &datagram, size_t offset = 0);

PUBLISHED:
  INLINE DatagramIterator();
  INLINE DatagramIterator(const Datagram &datagram, size_t offset = 0);

  INLINE bool get_bool();
  INLINE int8_t get_int8();
  INLINE uint8_t get_uint8();

  INLINE int16_t get_int16();
  INLINE int32_t get_int32();
  INLINE int64_t get_int64();
  INLINE uint16_t get_uint16();
  INLINE uint32_t get_uint32();
  INLINE uint64_t get_uint64();
  INLINE PN_float32 get_float32();
  INLINE PN_float64 get_float64();
  INLINE PN_stdfloat get_stdfloat();

  INLINE int16_t get_be_int16();
  INLINE int32_t get_be_int32();
  INLINE int64_t get_be_int64();
  INLINE uint16_t get_be_uint16();
  INLINE uint32_t get_be_uint32();
  INLINE uint64_t get_be_uint64();
  INLINE PN_float32 get_be_float32();
  INLINE PN_float64 get_be_float64();

  std::string get_string();
  std::string get_string32();
  std::string get_z_string();
  std::string get_fixed_string(size_t size);
  std::wstring get_wstring();

  INLINE vector_uchar get_blob();
  INLINE vector_uchar get_blob32();

  INLINE void skip_bytes(size_t size);
  vector_uchar extract_bytes(size_t size);
  size_t extract_bytes(unsigned char *into, size_t size);

  INLINE vector_uchar get_remaining_bytes() const;
  INLINE size_t get_remaining_size() const;

  INLINE const Datagram &get_datagram() const;
  INLINE size_t get_current_index() const;

  void output(std::ostream &out) const;
  void write(std::ostream &out, unsigned int indent=0) const;

private:
  const Datagram *_datagram;
  size_t _current_index;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "DatagramIterator");
  }

private:
  static TypeHandle _type_handle;
};

// These generic functions are primarily for reading a value from a datagram
// from within a template in which the actual type of the value is not known.
// If you do know the type, it's preferable to use the explicit get_*() method
// from above instead.

INLINE void
generic_read_datagram(bool &result, DatagramIterator &source);
INLINE void
generic_read_datagram(int &result, DatagramIterator &source);
INLINE void
generic_read_datagram(float &result, DatagramIterator &source);
INLINE void
generic_read_datagram(double &result, DatagramIterator &source);
INLINE void
generic_read_datagram(std::string &result, DatagramIterator &source);
INLINE void
generic_read_datagram(std::wstring &result, DatagramIterator &source);

#include "datagramIterator.I"

#endif
