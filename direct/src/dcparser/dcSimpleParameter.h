// Filename: dcSimpleParameter.h
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

#ifndef DCSIMPLEPARAMETER_H
#define DCSIMPLEPARAMETER_H

#include "dcbase.h"
#include "dcParameter.h"
#include "dcSubatomicType.h"

////////////////////////////////////////////////////////////////////
//       Class : DCSimpleParameter
// Description : This is the most fundamental kind of parameter type:
//               a single number or string, one of the DCSubatomicType
//               elements.  It may also optionally have a divisor,
//               which is meaningful only for the numeric type
//               elements (and represents a fixed-point numeric
//               convention).
////////////////////////////////////////////////////////////////////
class EXPCL_DIRECT DCSimpleParameter : public DCParameter {
public:
  DCSimpleParameter(DCSubatomicType type, int divisor = 1);
  DCSimpleParameter(const DCSimpleParameter &copy);

PUBLISHED:
  virtual DCSimpleParameter *as_simple_parameter();
  virtual DCParameter *make_copy() const;

  DCSubatomicType get_type() const;
  int get_divisor() const;

public:
  bool set_divisor(int divisor);

  virtual int calc_num_nested_fields(size_t length_bytes) const;
  virtual DCPackerInterface *get_nested_field(int n) const;

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

  virtual void output_instance(ostream &out, const string &prename, 
                               const string &name, const string &postname) const;
  virtual void generate_hash(HashGenerator &hash) const;

private:
  static DCSimpleParameter *create_nested_field(DCSubatomicType type, int divisor);
  static DCPackerInterface *create_uint32uint8_type();

private:
  DCSubatomicType _type;
  int _divisor;

  DCSubatomicType _nested_type;
  DCPackerInterface *_nested_field;
  size_t _bytes_per_element;

  // The rest of this is to maintain the static list of
  // DCPackerInterface objects for _nested_field, above.  We allocate
  // each possible object once, and don't delete it.
  typedef pmap<int, DCSimpleParameter *> DivisorMap;
  typedef pmap<DCSubatomicType, DivisorMap> NestedFieldMap;
  static NestedFieldMap _nested_field_map;

  class Uint32Uint8Type : public DCPackerInterface {
  public:
    Uint32Uint8Type();
    virtual DCPackerInterface *get_nested_field(int n) const;

    DCSimpleParameter *_uint32_type;
    DCSimpleParameter *_uint8_type;
  };

  static Uint32Uint8Type *_uint32uint8_type;
};

#endif
