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

  void begin(DCPackerInterface *root);
  bool end();

  INLINE int get_num_nested_fields() const;
  void push();
  void pop();

  INLINE DCSubatomicType get_pack_type() const;
  INLINE void pack_value(double value);
  INLINE void pack_value(int value);
  INLINE void pack_value(PN_int64 value);
  INLINE void pack_value(const string &value);
  INLINE void pack_literal_value(const string &value);

  INLINE bool had_pack_error() const;

  INLINE string get_string() const;
  INLINE size_t get_length() const;
public:
  INLINE const char *get_data() const;

private:
  INLINE void advance();

private:
  DCPackData _pack_data;

  class StackElement {
  public:
    DCPackerInterface *_current_parent;
    int _current_field_index;
    size_t _push_start;
  };
  typedef vector<StackElement> Stack;

  Stack _stack;
  DCPackerInterface *_current_field;
  DCPackerInterface *_current_parent;
  int _current_field_index;
  size_t _push_start;
  int _num_nested_fields;

  bool _pack_error;

  friend class DCPackerInterface;
};

#include "dcPacker.I"

#endif
