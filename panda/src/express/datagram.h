// Filename: datagram.h
// Created by:  drose (06Jun00)
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

#ifndef DATAGRAM_H
#define DATAGRAM_H

#include "pandabase.h"

#include "numeric_types.h"
#include "typedObject.h"
#include "littleEndian.h"
#include "bigEndian.h"
#include "pta_uchar.h"

////////////////////////////////////////////////////////////////////
//       Class : Datagram
// Description : An ordered list of data elements, formatted in memory
//               for transmission over a socket or writing to a data
//               file.
//
//               Data elements should be added one at a time, in
//               order, to the Datagram.  The nature and contents of
//               the data elements are totally up to the user.  When a
//               Datagram has been transmitted and received, its data
//               elements may be extracted using a DatagramIterator;
//               it is up to the caller to know the correct type of
//               each data element in order.
//
//               A Datagram is itself headerless; it is simply a
//               collection of data elements.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS Datagram : public TypedObject {
PUBLISHED:
  INLINE Datagram();
  INLINE Datagram(const void *data, size_t size);
  INLINE Datagram(const string &data);
  INLINE Datagram(const Datagram &copy);
  INLINE void operator = (const Datagram &copy);

  virtual ~Datagram();

  virtual void clear();
  void dump_hex(ostream &out) const;

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
  INLINE void add_z_string(string str);
  INLINE void add_fixed_string(const string &str, size_t size);

  void pad_bytes(size_t size);
  void append_data(const void *data, size_t size);
  INLINE void append_data(const string &data);

  INLINE string get_message() const;
  INLINE const void *get_data() const;
  INLINE size_t get_length() const;

  INLINE bool operator == (const Datagram &other) const;
  INLINE bool operator != (const Datagram &other) const;
  INLINE bool operator < (const Datagram &other) const;

private:
  PTA_uchar _data;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "Datagram",
                  TypedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "datagram.I"

#endif
