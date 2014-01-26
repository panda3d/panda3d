// Filename: datagramIterator.h
// Created by:  jns (07Feb00)
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

#ifndef DATAGRAMITERATOR_H
#define DATAGRAMITERATOR_H

#include "pandabase.h"

#include "datagram.h"
#include "numeric_types.h"

////////////////////////////////////////////////////////////////////
//       Class : DatagramIterator
// Description : A class to retrieve the individual data elements
//               previously stored in a Datagram.  Elements may be
//               retrieved one at a time; it is up to the caller to
//               know the correct type and order of each element.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS DatagramIterator {
public:
  INLINE void assign(Datagram &datagram, size_t offset = 0);

PUBLISHED:
  INLINE DatagramIterator();
  INLINE DatagramIterator(const Datagram &datagram, size_t offset = 0);
  INLINE DatagramIterator(const DatagramIterator &copy);
  INLINE void operator = (const DatagramIterator &copy);
  INLINE ~DatagramIterator();

  INLINE bool get_bool();
  INLINE PN_int8 get_int8();
  INLINE PN_uint8 get_uint8();

  INLINE PN_int16 get_int16();
  INLINE PN_int32 get_int32();
  INLINE PN_int64 get_int64();
  INLINE PN_uint16 get_uint16();
  INLINE PN_uint32 get_uint32();
  INLINE PN_uint64 get_uint64();
  INLINE PN_float32 get_float32();
  INLINE PN_float64 get_float64();
  INLINE PN_stdfloat get_stdfloat();

  INLINE PN_int16 get_be_int16();
  INLINE PN_int32 get_be_int32();
  INLINE PN_int64 get_be_int64();
  INLINE PN_uint16 get_be_uint16();
  INLINE PN_uint32 get_be_uint32();
  INLINE PN_uint64 get_be_uint64();
  INLINE PN_float32 get_be_float32();
  INLINE PN_float64 get_be_float64();

  string get_string();
  string get_string32();
  string get_z_string();
  string get_fixed_string(size_t size);
  wstring get_wstring();

  INLINE void skip_bytes(size_t size);
  string extract_bytes(size_t size);
  size_t extract_bytes(unsigned char *into, size_t size);

  INLINE string get_remaining_bytes() const;
  INLINE int get_remaining_size() const;

  INLINE const Datagram &get_datagram() const;
  INLINE size_t get_current_index() const;

  void output(ostream &out) const;
  void write(ostream &out, unsigned int indent=0) const;

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

// These generic functions are primarily for reading a value from a
// datagram from within a template in which the actual type of the
// value is not known.  If you do know the type, it's preferable to
// use the explicit get_*() method from above instead.

INLINE void
generic_read_datagram(bool &result, DatagramIterator &source);
INLINE void
generic_read_datagram(int &result, DatagramIterator &source);
INLINE void
generic_read_datagram(float &result, DatagramIterator &source);
INLINE void
generic_read_datagram(double &result, DatagramIterator &source);
INLINE void
generic_read_datagram(string &result, DatagramIterator &source);
INLINE void
generic_read_datagram(wstring &result, DatagramIterator &source);

#include "datagramIterator.I"

#endif
