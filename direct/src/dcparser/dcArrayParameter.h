/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcArrayParameter.h
 * @author drose
 * @date 2004-06-17
 */

#ifndef DCARRAYPARAMETER_H
#define DCARRAYPARAMETER_H

#include "dcbase.h"
#include "dcParameter.h"
#include "dcNumericRange.h"

/**
 * This represents an array of some other kind of object, meaning this
 * parameter type accepts an arbitrary (or possibly fixed) number of nested
 * fields, all of which are of the same type.
 */
class EXPCL_DIRECT_DCPARSER DCArrayParameter : public DCParameter {
public:
  DCArrayParameter(DCParameter *element_type,
                   const DCUnsignedIntRange &size = DCUnsignedIntRange());
  DCArrayParameter(const DCArrayParameter &copy);
  virtual ~DCArrayParameter();

PUBLISHED:
  virtual DCArrayParameter *as_array_parameter();
  virtual const DCArrayParameter *as_array_parameter() const;
  virtual DCParameter *make_copy() const;
  virtual bool is_valid() const;

  DCParameter *get_element_type() const;
  int get_array_size() const;

public:
  virtual DCParameter *append_array_specification(const DCUnsignedIntRange &size);

  virtual int calc_num_nested_fields(size_t length_bytes) const;
  virtual DCPackerInterface *get_nested_field(int n) const;
  virtual bool validate_num_nested_fields(int num_nested_fields) const;

  virtual void output_instance(std::ostream &out, bool brief, const std::string &prename,
                               const std::string &name, const std::string &postname) const;
  virtual void generate_hash(HashGenerator &hashgen) const;
  virtual void pack_string(DCPackData &pack_data, const std::string &value,
                           bool &pack_error, bool &range_error) const;
  virtual void pack_blob(DCPackData &pack_data, const vector_uchar &value,
                         bool &pack_error, bool &range_error) const;
  virtual bool pack_default_value(DCPackData &pack_data, bool &pack_error) const;
  virtual void unpack_string(const char *data, size_t length, size_t &p,
                             std::string &value, bool &pack_error, bool &range_error) const;
  virtual void unpack_blob(const char *data, size_t length, size_t &p,
                           vector_uchar &value, bool &pack_error, bool &range_error) const;

protected:
  virtual bool do_check_match(const DCPackerInterface *other) const;
  virtual bool do_check_match_simple_parameter(const DCSimpleParameter *other) const;
  virtual bool do_check_match_class_parameter(const DCClassParameter *other) const;
  virtual bool do_check_match_array_parameter(const DCArrayParameter *other) const;

private:
  DCParameter *_element_type;
  int _array_size;
  DCUnsignedIntRange _array_size_range;
};

#endif
