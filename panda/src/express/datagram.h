/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file datagram.h
 * @author drose
 * @date 2000-06-06
 */

#ifndef DATAGRAM_H
#define DATAGRAM_H

#include "pandabase.h"

#include "numeric_types.h"
#include "typedObject.h"
#include "littleEndian.h"
#include "bigEndian.h"
#include "pta_uchar.h"

/**
 * An ordered list of data elements, formatted in memory for transmission over
 * a socket or writing to a data file.
 *
 * Data elements should be added one at a time, in order, to the Datagram.
 * The nature and contents of the data elements are totally up to the user.
 * When a Datagram has been transmitted and received, its data elements may be
 * extracted using a DatagramIterator; it is up to the caller to know the
 * correct type of each data element in order.
 *
 * A Datagram is itself headerless; it is simply a collection of data
 * elements.
 */
class EXPCL_PANDA_EXPRESS Datagram : public TypedObject {
PUBLISHED:
  INLINE Datagram() = default;
  INLINE Datagram(const void *data, size_t size);
  INLINE explicit Datagram(vector_uchar data);
  Datagram(const Datagram &copy) = default;
  Datagram(Datagram &&from) noexcept = default;
  virtual ~Datagram();

  Datagram &operator = (const Datagram &copy) = default;
  Datagram &operator = (Datagram &&from) noexcept = default;

  virtual void clear();
  void dump_hex(std::ostream &out, unsigned int indent=0) const;

  INLINE void add_bool(bool value);
  INLINE void add_int8(int8_t value);
  INLINE void add_uint8(uint8_t value);

  // The default numeric packing is little-endian.
  INLINE void add_int16(int16_t value);
  INLINE void add_int32(int32_t value);
  INLINE void add_int64(int64_t value);
  INLINE void add_uint16(uint16_t value);
  INLINE void add_uint32(uint32_t value);
  INLINE void add_uint64(uint64_t value);
  INLINE void add_float32(PN_float32 value);
  INLINE void add_float64(PN_float64 value);
  INLINE void add_stdfloat(PN_stdfloat value);

  // These functions pack numbers big-endian, in case that's desired.
  INLINE void add_be_int16(int16_t value);
  INLINE void add_be_int32(int32_t value);
  INLINE void add_be_int64(int64_t value);
  INLINE void add_be_uint16(uint16_t value);
  INLINE void add_be_uint32(uint32_t value);
  INLINE void add_be_uint64(uint64_t value);
  INLINE void add_be_float32(PN_float32 value);
  INLINE void add_be_float64(PN_float64 value);

  INLINE void add_string(const std::string &str);
  INLINE void add_string32(const std::string &str);
  INLINE void add_z_string(const std::string &str);
  INLINE void add_fixed_string(const std::string &str, size_t size);
  void add_wstring(const std::wstring &str);

  INLINE void add_blob(const vector_uchar &);
  INLINE void add_blob32(const vector_uchar &);

  void pad_bytes(size_t size);
  void append_data(const void *data, size_t size);
  INLINE void append_data(const vector_uchar &data);

public:
  void assign(const void *data, size_t size);

  INLINE std::string get_message() const;
  INLINE const void *get_data() const;

PUBLISHED:
  EXTENSION(INLINE PyObject *get_message() const);
  EXTENSION(INLINE PyObject *__bytes__() const);

  INLINE size_t get_length() const;

  INLINE void set_array(PTA_uchar data);
  INLINE void copy_array(CPTA_uchar data);
  INLINE CPTA_uchar get_array() const;
  INLINE PTA_uchar modify_array();

  INLINE void set_stdfloat_double(bool stdfloat_double);
  INLINE bool get_stdfloat_double() const;

  INLINE bool operator == (const Datagram &other) const;
  INLINE bool operator != (const Datagram &other) const;
  INLINE bool operator < (const Datagram &other) const;

  void output(std::ostream &out) const;
  void write(std::ostream &out, unsigned int indent=0) const;

private:
  PTA_uchar _data;

#ifdef STDFLOAT_DOUBLE
  bool _stdfloat_double = true;
#else
  bool _stdfloat_double = false;
#endif

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

// These generic functions are primarily for writing a value to a datagram
// from within a template in which the actual type of the value is not known.
// If you do know the type, it's preferable to use the explicit add_*() method
// from above instead.

INLINE void
generic_write_datagram(Datagram &dest, bool value);
INLINE void
generic_write_datagram(Datagram &dest, int value);
INLINE void
generic_write_datagram(Datagram &dest, float value);
INLINE void
generic_write_datagram(Datagram &dest, double value);
INLINE void
generic_write_datagram(Datagram &dest, const std::string &value);
INLINE void
generic_write_datagram(Datagram &dest, const std::wstring &value);
INLINE void
generic_write_datagram(Datagram &dest, const vector_uchar &value);


#include "datagram.I"

#endif
