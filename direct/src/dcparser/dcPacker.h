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

////////////////////////////////////////////////////////////////////
//       Class : DCPacker
// Description : This class can be used for packing a series of
//               numeric and string data into a binary stream,
//               according to the DC specification.
////////////////////////////////////////////////////////////////////
class EXPCL_DIRECT DCPacker {
PUBLISHED:
  DCPacker();
  ~DCPacker();

  void begin_pack(const DCPackerInterface *root);
  bool end_pack();

public:
  void begin_unpack(const char *data, size_t length,
                    const DCPackerInterface *root);
PUBLISHED:
  void begin_unpack(const string &data, const DCPackerInterface *root);
  bool end_unpack();

  INLINE bool has_nested_fields() const;
  INLINE int get_num_nested_fields() const;
  INLINE bool more_nested_fields() const;

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

#ifdef HAVE_PYTHON
  void pack_object(PyObject *object);
  PyObject *unpack_object();
#endif

  bool parse_and_pack(const string &formatted_object);
  bool parse_and_pack(istream &in);
  string unpack_and_format();
  void unpack_and_format(ostream &out);

  INLINE bool had_pack_error() const;
  INLINE size_t get_num_unpacked_bytes() const;

  INLINE string get_string() const;
  INLINE size_t get_length() const;
public:
  INLINE const char *get_data() const;

private:
  INLINE void advance();

private:
  enum Mode {
    M_idle,
    M_pack,
    M_unpack,
  };
  Mode _mode;

  DCPackData _pack_data;
  string _unpack_str;
  const char *_unpack_data;
  size_t _unpack_length;
  size_t _unpack_p;

  class StackElement {
  public:
    const DCPackerInterface *_current_parent;
    int _current_field_index;
    size_t _push_marker;
  };
  typedef pvector<StackElement> Stack;

  Stack _stack;
  const DCPackerInterface *_current_field;
  const DCPackerInterface *_current_parent;
  int _current_field_index;

  // In pack mode, _push_marker marks the beginning of the push record
  // (so we can go back and write in the length later).  In unpack
  // mode, it marks the end of the push record (so we know when we've
  // reached the end).
  size_t _push_marker;
  int _num_nested_fields;

  bool _pack_error;
};

#include "dcPacker.I"

#endif
