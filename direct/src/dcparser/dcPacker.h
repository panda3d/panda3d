// Filename: dcPacker.h
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

#ifndef DCPACKER_H
#define DCPACKER_H

#include "dcbase.h"
#include "dcPackerInterface.h"
#include "dcSubatomicType.h"
#include "dcPackData.h"
#include "dcPackerCatalog.h"
#include "dcPython.h"

class DCClass;
class DCSwitchParametera;

////////////////////////////////////////////////////////////////////
//       Class : DCPacker
// Description : This class can be used for packing a series of
//               numeric and string data into a binary stream,
//               according to the DC specification.
//
//               See also direct/src/doc/dcPacker.txt for a more
//               complete description and examples of using this
//               class.
////////////////////////////////////////////////////////////////////
class EXPCL_DIRECT DCPacker {
PUBLISHED:
  DCPacker();
  ~DCPacker();

  INLINE void clear_data();

  void begin_pack(const DCPackerInterface *root);
  bool end_pack();

  void set_unpack_data(const string &data);
public:
  void set_unpack_data(const char *unpack_data, size_t unpack_length, 
                       bool owns_unpack_data);

PUBLISHED:
  void begin_unpack(const DCPackerInterface *root);
  bool end_unpack();

  void begin_repack(const DCPackerInterface *root);
  bool end_repack();

  bool seek(const string &field_name);

  INLINE bool has_nested_fields() const;
  INLINE int get_num_nested_fields() const;
  INLINE bool more_nested_fields() const;

  INLINE const DCPackerInterface *get_current_parent() const;
  INLINE const DCPackerInterface *get_current_field() const;
  INLINE const DCSwitchParameter *get_last_switch() const;
  INLINE DCPackType get_pack_type() const;

  void push();
  void pop();

  INLINE void pack_double(double value);
  INLINE void pack_int(int value);
  INLINE void pack_uint(unsigned int value);
  INLINE void pack_int64(PN_int64 value);
  INLINE void pack_uint64(PN_uint64 value);
  INLINE void pack_string(const string &value);
  INLINE void pack_literal_value(const string &value);

  INLINE double unpack_double();
  INLINE int unpack_int();
  INLINE unsigned int unpack_uint();
  INLINE PN_int64 unpack_int64();
  INLINE PN_uint64 unpack_uint64();
  INLINE string unpack_string();
  INLINE string unpack_literal_value();
  void unpack_validate();
  void unpack_skip();

public:
  // The following are variants on the above unpack() calls that pass
  // the result back by reference instead of as a return value.
  INLINE void unpack_double(double &value);
  INLINE void unpack_int(int &value);
  INLINE void unpack_uint(unsigned int &value);
  INLINE void unpack_int64(PN_int64 &value);
  INLINE void unpack_uint64(PN_uint64 &value);
  INLINE void unpack_string(string &value);
  INLINE void unpack_literal_value(string &value);

PUBLISHED:

#ifdef HAVE_PYTHON
  void pack_object(PyObject *object);
  PyObject *unpack_object();
#endif

  bool parse_and_pack(const string &formatted_object);
  bool parse_and_pack(istream &in);
  string unpack_and_format();
  void unpack_and_format(ostream &out);

  INLINE bool had_pack_error() const;
  INLINE bool had_range_error() const;
  INLINE bool had_error() const;
  INLINE size_t get_num_unpacked_bytes() const;

  INLINE string get_string() const;
  INLINE size_t get_length() const;
public:
  INLINE const char *get_data() const;

  INLINE void append_data(const char *buffer, size_t size);
  INLINE char *get_write_pointer(size_t size);

PUBLISHED:
  // The following methods are used only for packing (or unpacking)
  // raw data into the buffer between packing sessions (e.g. between
  // calls to end_pack() and the next begin_pack()).

  INLINE void raw_pack_int8(int value);
  INLINE void raw_pack_int16(int value);
  INLINE void raw_pack_int32(int value);
  INLINE void raw_pack_int64(PN_int64 value);
  INLINE void raw_pack_uint8(unsigned int value);
  INLINE void raw_pack_uint16(unsigned int value);
  INLINE void raw_pack_uint32(unsigned int value);
  INLINE void raw_pack_uint64(PN_uint64 value);
  INLINE void raw_pack_float64(double value);
  INLINE void raw_pack_string(const string &value);

  INLINE int raw_unpack_int8();
  INLINE int raw_unpack_int16();
  INLINE int raw_unpack_int32();
  INLINE PN_int64 raw_unpack_int64();
  INLINE unsigned int raw_unpack_uint8();
  INLINE unsigned int raw_unpack_uint16();
  INLINE unsigned int raw_unpack_uint32();
  INLINE PN_uint64 raw_unpack_uint64();
  INLINE double raw_unpack_float64();
  INLINE string raw_unpack_string();

public:
  INLINE void raw_unpack_int8(int &value);
  INLINE void raw_unpack_int16(int &value);
  INLINE void raw_unpack_int32(int &value);
  INLINE void raw_unpack_int64(PN_int64 &value);
  INLINE void raw_unpack_uint8(unsigned int &value);
  INLINE void raw_unpack_uint16(unsigned int &value);
  INLINE void raw_unpack_uint32(unsigned int &value);
  INLINE void raw_unpack_uint64(PN_uint64 &value);
  INLINE void raw_unpack_float64(double &value);
  INLINE void raw_unpack_string(string &value);

public:
  static void enquote_string(ostream &out, char quote_mark, const string &str);
  static void output_hex_string(ostream &out, const string &str);

private:
  INLINE void advance();
  void handle_switch(const DCSwitchParameter *switch_parameter);
  void clear();

#ifdef HAVE_PYTHON
  void pack_class_object(DCClass *dclass, PyObject *object);
  PyObject *unpack_class_object(DCClass *dclass);
  void set_class_element(PyObject *object, const DCField *field);
#endif

private:
  enum Mode {
    M_idle,
    M_pack,
    M_unpack,
    M_repack,
  };
  Mode _mode;

  DCPackData _pack_data;
  const char *_unpack_data;
  size_t _unpack_length;
  bool _owns_unpack_data;
  size_t _unpack_p;

  const DCPackerInterface *_root;
  const DCPackerCatalog *_catalog;
  const DCPackerCatalog::LiveCatalog *_live_catalog;

  class StackElement {
  public:
    const DCPackerInterface *_current_parent;
    int _current_field_index;
    size_t _push_marker;
    size_t _pop_marker;
  };
  typedef pvector<StackElement> Stack;

  Stack _stack;
  const DCPackerInterface *_current_field;
  const DCPackerInterface *_current_parent;
  int _current_field_index;

  // _push_marker marks the beginning of the push record (so we can go
  // back and write in the length later, or figure out the switch
  // parameter).
  size_t _push_marker;
  // _pop_marker is used in unpack mode with certain data structures
  // (like dynamic arrays) to mark the end of the push record (so we
  // know when we've reached the end).  It is zero when it is not in
  // use.
  size_t _pop_marker;
  int _num_nested_fields;
  const DCSwitchParameter *_last_switch;

  bool _pack_error;
  bool _range_error;
};

#include "dcPacker.I"

#endif
