// Filename: dcSimpleType.h
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

#ifndef DCSIMPLETYPE_H
#define DCSIMPLETYPE_H

#include "dcbase.h"
#include "dcType.h"
#include "dcSubatomicType.h"

////////////////////////////////////////////////////////////////////
//       Class : DCSimpleType
// Description : This is the most fundamental kind of parameter type:
//               a single number or string, one of the DCSubatomicType
//               elements.  It may also optionally have a divisor,
//               which is meaningful only for the numeric type
//               elements (and represents a fixed-point numeric
//               convention).
////////////////////////////////////////////////////////////////////
class EXPCL_DIRECT DCSimpleType : public DCType {
public:
  DCSimpleType(DCSubatomicType type, int divisor = 1);

PUBLISHED:
  virtual DCSimpleType *as_simple_type();
  virtual DCType *make_copy() const;

  DCSubatomicType get_type() const;
  int get_divisor() const;
  void set_divisor(int divisor);

public:
  virtual bool has_nested_fields() const;
  virtual int get_num_nested_fields() const;
  virtual DCPackerInterface *get_nested_field(int n) const;
  virtual size_t get_length_bytes() const;

  virtual DCSubatomicType get_pack_type() const;
  virtual bool pack_double(DCPackData &pack_data, double value) const;
  virtual bool pack_int(DCPackData &pack_data, int value) const;
  virtual bool pack_int64(DCPackData &pack_data, PN_int64 value) const;
  virtual bool pack_string(DCPackData &pack_data, const string &value) const;

  virtual void output(ostream &out, const string &parameter_name, 
                      bool brief) const;
  virtual void generate_hash(HashGenerator &hash) const;

private:
  static DCSimpleType *create_nested_field(DCSubatomicType type, int divisor);
  static DCPackerInterface *create_uint32uint8_type();

#ifdef HAVE_PYTHON
public:
  virtual void pack_arg(Datagram &datagram, PyObject *item) const;
  virtual PyObject *unpack_arg(DatagramIterator &iterator) const;

private:
  void do_pack_arg(Datagram &datagram, PyObject *item, DCSubatomicType type) const;
  PyObject *do_unpack_arg(DatagramIterator &iterator, DCSubatomicType type) const;
#endif  // HAVE_PYTHON

private:
  DCSubatomicType _type;
  int _divisor;

  bool _is_array;
  DCSubatomicType _nested_type;
  DCPackerInterface *_nested_field;

  // The rest of this is to maintain the static list of
  // DCPackerInterface objects for _nested_field, above.  We allocate
  // each possible object once, and don't delete it.
  typedef pmap<int, DCSimpleType *> DivisorMap;
  typedef pmap<DCSubatomicType, DivisorMap> NestedFieldMap;
  static NestedFieldMap _nested_field_map;

  class Uint32Uint8Type : public DCPackerInterface {
  public:
    Uint32Uint8Type();
    virtual bool has_nested_fields() const;
    virtual int get_num_nested_fields() const;
    virtual DCPackerInterface *get_nested_field(int n) const;

    DCSimpleType *_uint32_type;
    DCSimpleType *_uint8_type;
  };

  static Uint32Uint8Type *_uint32uint8_type;
};

#endif
