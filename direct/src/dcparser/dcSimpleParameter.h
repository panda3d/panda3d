/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcSimpleParameter.h
 * @author drose
 * @date 2004-06-15
 */

#ifndef DCSIMPLEPARAMETER_H
#define DCSIMPLEPARAMETER_H

#include "dcbase.h"
#include "dcParameter.h"
#include "dcSubatomicType.h"
#include "dcNumericRange.h"

/**
 * This is the most fundamental kind of parameter type: a single number or
 * string, one of the DCSubatomicType elements.  It may also optionally have a
 * divisor, which is meaningful only for the numeric type elements (and
 * represents a fixed-point numeric convention).
 */
class EXPCL_DIRECT_DCPARSER DCSimpleParameter : public DCParameter {
public:
  DCSimpleParameter(DCSubatomicType type, unsigned int divisor = 1);
  DCSimpleParameter(const DCSimpleParameter &copy);

PUBLISHED:
  virtual DCSimpleParameter *as_simple_parameter();
  virtual const DCSimpleParameter *as_simple_parameter() const;
  virtual DCParameter *make_copy() const;
  virtual bool is_valid() const;

  DCSubatomicType get_type() const;
  bool has_modulus() const;
  double get_modulus() const;
  int get_divisor() const;

public:
  bool is_numeric_type() const;
  bool set_modulus(double modulus);
  bool set_divisor(unsigned int divisor);
  bool set_range(const DCDoubleRange &range);

  virtual int calc_num_nested_fields(size_t length_bytes) const;
  virtual DCPackerInterface *get_nested_field(int n) const;

  virtual void pack_double(DCPackData &pack_data, double value,
                           bool &pack_error, bool &range_error) const;
  virtual void pack_int(DCPackData &pack_data, int value,
                        bool &pack_error, bool &range_error) const;
  virtual void pack_uint(DCPackData &pack_data, unsigned int value,
                         bool &pack_error, bool &range_error) const;
  virtual void pack_int64(DCPackData &pack_data, int64_t value,
                          bool &pack_error, bool &range_error) const;
  virtual void pack_uint64(DCPackData &pack_data, uint64_t value,
                           bool &pack_error, bool &range_error) const;
  virtual void pack_string(DCPackData &pack_data, const std::string &value,
                           bool &pack_error, bool &range_error) const;
  virtual void pack_blob(DCPackData &pack_data, const vector_uchar &value,
                         bool &pack_error, bool &range_error) const;
  virtual bool pack_default_value(DCPackData &pack_data, bool &pack_error) const;

  virtual void unpack_double(const char *data, size_t length, size_t &p,
                             double &value, bool &pack_error, bool &range_error) const;
  virtual void unpack_int(const char *data, size_t length, size_t &p,
                          int &value, bool &pack_error, bool &range_error) const;
  virtual void unpack_uint(const char *data, size_t length, size_t &p,
                           unsigned int &value, bool &pack_error, bool &range_error) const;
  virtual void unpack_int64(const char *data, size_t length, size_t &p,
                            int64_t &value, bool &pack_error, bool &range_error) const;
  virtual void unpack_uint64(const char *data, size_t length, size_t &p,
                             uint64_t &value, bool &pack_error, bool &range_error) const;
  virtual void unpack_string(const char *data, size_t length, size_t &p,
                             std::string &value, bool &pack_error, bool &range_error) const;
  virtual void unpack_blob(const char *data, size_t length, size_t &p,
                           vector_uchar &value, bool &pack_error, bool &range_error) const;
  virtual bool unpack_validate(const char *data, size_t length, size_t &p,
                               bool &pack_error, bool &range_error) const;
  virtual bool unpack_skip(const char *data, size_t length, size_t &p,
                           bool &pack_error) const;

  virtual void output_instance(std::ostream &out, bool brief, const std::string &prename,
                               const std::string &name, const std::string &postname) const;
  virtual void generate_hash(HashGenerator &hashgen) const;

protected:
  virtual bool do_check_match(const DCPackerInterface *other) const;
  virtual bool do_check_match_simple_parameter(const DCSimpleParameter *other) const;
  virtual bool do_check_match_array_parameter(const DCArrayParameter *other) const;

private:
  static DCSimpleParameter *create_nested_field(DCSubatomicType type,
                                                unsigned int divisor);
  static DCPackerInterface *create_uint32uint8_type();

private:
  DCSubatomicType _type;
  unsigned int _divisor;

  DCSubatomicType _nested_type;
  DCPackerInterface *_nested_field;
  size_t _bytes_per_element;

  // The rest of this is to maintain the static list of DCPackerInterface
  // objects for _nested_field, above.  We allocate each possible object once,
  // and don't delete it.
  typedef pmap<unsigned int, DCSimpleParameter *> DivisorMap;
  typedef pmap<DCSubatomicType, DivisorMap> NestedFieldMap;
  static NestedFieldMap _nested_field_map;

  // These are the range and modulus values as specified by the user, unscaled
  // by the divisor.
  DCDoubleRange _orig_range;
  bool _has_modulus;
  double _orig_modulus;

  // Only the range appropriate to this type will be filled in.
  DCIntRange _int_range;
  DCUnsignedIntRange _uint_range;
  DCInt64Range _int64_range;
  DCUnsignedInt64Range _uint64_range;
  DCDoubleRange _double_range;

  // All of these modulus values will be filled in, regardless of the type.
  unsigned int _uint_modulus;
  uint64_t _uint64_modulus;
  double _double_modulus;

  static DCClassParameter *_uint32uint8_type;
};

#endif
