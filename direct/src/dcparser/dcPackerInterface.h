// Filename: dcPackerInterface.h
// Created by:  drose (15Jun04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef DCPACKERINTERFACE_H
#define DCPACKERINTERFACE_H

#include "dcbase.h"
#include "dcSubatomicType.h"

class DCPackData;

BEGIN_PUBLISH
// This enumerated type is returned by get_pack_type() and represents
// the best choice for a subsequent call to pack_*() or unpack_*().
enum DCPackType {
  // This one should never be returned in a normal situation.
  PT_invalid,

  // These PackTypes are all fundamental types, and should be packed
  // (or unpacked) with the corresponding call to pack_double(),
  // pack_int(), etc.
  PT_double,
  PT_int,
  PT_uint,
  PT_int64,
  PT_uint64,
  PT_string,

  // The remaining PackTypes imply a need to call push() and pop().
  // They are all variants on the same thing: a list of nested fields,
  // but the PackType provides a bit of a semantic context.
  PT_array,
  PT_field,
  PT_class,
};
END_PUBLISH

////////////////////////////////////////////////////////////////////
//       Class : DCPackerInterface
// Description : This defines the internal interface for packing
//               values into a DCField.  The various different DC
//               objects inherit from this.  
//
//               Normally these methods are called only by the
//               DCPacker object; the user wouldn't normally call
//               these directly.
////////////////////////////////////////////////////////////////////
class EXPCL_DIRECT DCPackerInterface {
public:
  DCPackerInterface(const string &name = string());
  DCPackerInterface(const DCPackerInterface &copy);
  virtual ~DCPackerInterface();

PUBLISHED:
  const string &get_name() const;
  void set_name(const string &name);

public:
  bool has_fixed_byte_size() const;
  size_t get_fixed_byte_size() const;
  size_t get_num_length_bytes() const;

  bool has_nested_fields() const;
  int get_num_nested_fields() const;
  virtual int calc_num_nested_fields(size_t length_bytes) const;
  virtual DCPackerInterface *get_nested_field(int n) const;

  DCPackType get_pack_type() const;

  virtual bool pack_double(DCPackData &pack_data, double value) const;
  virtual bool pack_int(DCPackData &pack_data, int value) const;
  virtual bool pack_uint(DCPackData &pack_data, unsigned int value) const;
  virtual bool pack_int64(DCPackData &pack_data, PN_int64 value) const;
  virtual bool pack_uint64(DCPackData &pack_data, PN_uint64 value) const;
  virtual bool pack_string(DCPackData &pack_data, const string &value) const;

  virtual bool unpack_double(const char *data, size_t length, size_t &p, double &value) const;
  virtual bool unpack_int(const char *data, size_t length, size_t &p, int &value) const;
  virtual bool unpack_uint(const char *data, size_t length, size_t &p, unsigned int &value) const;
  virtual bool unpack_int64(const char *data, size_t length, size_t &p, PN_int64 &value) const;
  virtual bool unpack_uint64(const char *data, size_t length, size_t &p, PN_uint64 &value) const;
  virtual bool unpack_string(const char *data, size_t length, size_t &p, string &value) const;

  // These are the low-level interfaces for packing and unpacking
  // numbers from a buffer.  You're responsible for making sure the
  // buffer has enough room, and for incrementing the pointer.
  INLINE static void do_pack_int8(char *buffer, int value);
  INLINE static void do_pack_int16(char *buffer, int value);
  INLINE static void do_pack_int32(char *buffer, int value);
  INLINE static void do_pack_int64(char *buffer, PN_int64 value);
  INLINE static void do_pack_uint8(char *buffer, unsigned int value);
  INLINE static void do_pack_uint16(char *buffer, unsigned int value);
  INLINE static void do_pack_uint32(char *buffer, unsigned int value);
  INLINE static void do_pack_uint64(char *buffer, PN_uint64 value);
  INLINE static void do_pack_float64(char *buffer, double value);

  INLINE static int do_unpack_int8(const char *buffer);
  INLINE static int do_unpack_int16(const char *buffer);
  INLINE static int do_unpack_int32(const char *buffer);
  INLINE static PN_int64 do_unpack_int64(const char *buffer);
  INLINE static unsigned int do_unpack_uint8(const char *buffer);
  INLINE static unsigned int do_unpack_uint16(const char *buffer);
  INLINE static unsigned int do_unpack_uint32(const char *buffer);
  INLINE static PN_uint64 do_unpack_uint64(const char *buffer);
  INLINE static double do_unpack_float64(const char *buffer);

protected:
  string _name;
  bool _has_fixed_byte_size;
  size_t _fixed_byte_size;
  size_t _num_length_bytes;
  bool _has_nested_fields;
  int _num_nested_fields;
  DCPackType _pack_type;
};

#include "dcPackerInterface.I"

#endif
