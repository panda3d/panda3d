/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcArrayParameter.cxx
 * @author drose
 * @date 2004-06-17
 */

#include "dcArrayParameter.h"
#include "dcSimpleParameter.h"
#include "dcClassParameter.h"
#include "hashGenerator.h"

using std::string;

/**
 *
 */
DCArrayParameter::
DCArrayParameter(DCParameter *element_type, const DCUnsignedIntRange &size) :
  _element_type(element_type),
  _array_size_range(size)
{
  set_name(_element_type->get_name());
  _element_type->set_name(string());

  _array_size = -1;
  if (_array_size_range.has_one_value()) {
    _array_size = _array_size_range.get_one_value();
  } else {
    _has_range_limits = true;
  }

  if (_array_size >= 0 && _element_type->has_fixed_byte_size()) {
    _has_fixed_byte_size = true;
    _fixed_byte_size = _array_size * _element_type->get_fixed_byte_size();
    _has_fixed_structure = true;

  } else {
    // We only need to store the length bytes if the array has a variable
    // size.
    _num_length_bytes = 2;
  }

  if (_element_type->has_range_limits()) {
    _has_range_limits = true;
  }

  if (_element_type->has_default_value()) {
    _has_default_value = true;
  }

  _has_nested_fields = true;
  _num_nested_fields = _array_size;
  _pack_type = PT_array;

  DCSimpleParameter *simple_type = _element_type->as_simple_parameter();
  if (simple_type != nullptr) {
    if (simple_type->get_type() == ST_char) {
      // We make a special case for char[] arrays: these we format as a
      // string.  (It will still accept an array of ints packed into it.)  We
      // don't make this special case for uint8[] or int8[] arrays, although
      // we will accept a string packed in for them.
      _pack_type = PT_string;
    }
  }
}

/**
 *
 */
DCArrayParameter::
DCArrayParameter(const DCArrayParameter &copy) :
  DCParameter(copy),
  _element_type(copy._element_type->make_copy()),
  _array_size(copy._array_size),
  _array_size_range(copy._array_size_range)
{
}

/**
 *
 */
DCArrayParameter::
~DCArrayParameter() {
  delete _element_type;
}

/**
 *
 */
DCArrayParameter *DCArrayParameter::
as_array_parameter() {
  return this;
}

/**
 *
 */
const DCArrayParameter *DCArrayParameter::
as_array_parameter() const {
  return this;
}

/**
 *
 */
DCParameter *DCArrayParameter::
make_copy() const {
  return new DCArrayParameter(*this);
}

/**
 * Returns false if the type is an invalid type (e.g.  declared from an
 * undefined typedef), true if it is valid.
 */
bool DCArrayParameter::
is_valid() const {
  return _element_type->is_valid();
}

/**
 * Returns the type of the individual elements of this array.
 */
DCParameter *DCArrayParameter::
get_element_type() const {
  return _element_type;
}

/**
 * Returns the fixed number of elements in this array, or -1 if the array may
 * contain a variable number of elements.
 */
int DCArrayParameter::
get_array_size() const {
  return _array_size;
}

/**
 * Returns the type represented by this_type[size].
 *
 * In the case of a DCArrayParameter, this means it modifies the current type
 * to append the array specification on the innermost type, and returns this
 * same pointer again.
 */
DCParameter *DCArrayParameter::
append_array_specification(const DCUnsignedIntRange &size) {
  if (get_typedef() != nullptr) {
    // If this was a typedef, wrap it directly.
    return new DCArrayParameter(this, size);
  }

  // Otherwise, the brackets get applied to the inner type.
  _element_type = _element_type->append_array_specification(size);
  return this;
}

/**
 * This flavor of get_num_nested_fields is used during unpacking.  It returns
 * the number of nested fields to expect, given a certain length in bytes (as
 * read from the get_num_length_bytes() stored in the stream on the pack).
 * This will only be called if get_num_length_bytes() returns nonzero.
 */
int DCArrayParameter::
calc_num_nested_fields(size_t length_bytes) const {
  if (_element_type->has_fixed_byte_size()) {
    return length_bytes / _element_type->get_fixed_byte_size();
  }
  return -1;
}

/**
 * Returns the DCPackerInterface object that represents the nth nested field.
 * This may return NULL if there is no such field (but it shouldn't do this if
 * n is in the range 0 <= n < get_num_nested_fields()).
 */
DCPackerInterface *DCArrayParameter::
get_nested_field(int) const {
  return _element_type;
}

/**
 * After a number of fields have been packed via push() .. pack_*() .. pop(),
 * this is called to confirm that the number of nested fields that were added
 * is valid for this type.  This is primarily useful for array types with
 * dynamic ranges that can't validate the number of fields any other way.
 */
bool DCArrayParameter::
validate_num_nested_fields(int num_nested_fields) const {
  bool range_error = false;
  _array_size_range.validate(num_nested_fields, range_error);

  return !range_error;
}

/**
 * Formats the parameter in the C++-like dc syntax as a typename and
 * identifier.
 */
void DCArrayParameter::
output_instance(std::ostream &out, bool brief, const string &prename,
                const string &name, const string &postname) const {
  if (get_typedef() != nullptr) {
    output_typedef_name(out, brief, prename, name, postname);

  } else {
    std::ostringstream strm;

    strm << "[";
    _array_size_range.output(strm);
    strm << "]";

    _element_type->output_instance(out, brief, prename, name,
                                   postname + strm.str());
  }
}

/**
 * Accumulates the properties of this type into the hash.
 */
void DCArrayParameter::
generate_hash(HashGenerator &hashgen) const {
  DCParameter::generate_hash(hashgen);
  _element_type->generate_hash(hashgen);
  _array_size_range.generate_hash(hashgen);
}

/**
 * Packs the indicated numeric or string value into the stream.
 */
void DCArrayParameter::
pack_string(DCPackData &pack_data, const string &value,
            bool &pack_error, bool &range_error) const {
  // We can only pack a string if the array element type is char or int8.
  DCSimpleParameter *simple_type = _element_type->as_simple_parameter();
  if (simple_type == nullptr) {
    pack_error = true;
    return;
  }

  size_t string_length = value.length();

  switch (simple_type->get_type()) {
  case ST_char:
  case ST_uint8:
  case ST_int8:
    _array_size_range.validate(string_length, range_error);
    if (_num_length_bytes != 0) {
      nassertv(_num_length_bytes == 2);
      do_pack_uint16(pack_data.get_write_pointer(2), string_length);
    }
    pack_data.append_data(value.data(), string_length);
    break;

  default:
    pack_error = true;
  }
}

/**
 * Packs the indicated numeric or string value into the stream.
 */
void DCArrayParameter::
pack_blob(DCPackData &pack_data, const vector_uchar &value,
          bool &pack_error, bool &range_error) const {
  // We can only pack a string if the array element type is char or int8.
  DCSimpleParameter *simple_type = _element_type->as_simple_parameter();
  if (simple_type == nullptr) {
    pack_error = true;
    return;
  }

  size_t blob_size = value.size();

  switch (simple_type->get_type()) {
  case ST_char:
  case ST_uint8:
  case ST_int8:
    _array_size_range.validate(blob_size, range_error);
    if (_num_length_bytes != 0) {
      nassertv(_num_length_bytes == 2);
      do_pack_uint16(pack_data.get_write_pointer(2), blob_size);
    }
    pack_data.append_data((const char *)value.data(), blob_size);
    break;

  default:
    pack_error = true;
  }
}

/**
 * Packs the arrayParameter's specified default value (or a sensible default
 * if no value is specified) into the stream.  Returns true if the default
 * value is packed, false if the arrayParameter doesn't know how to pack its
 * default value.
 */
bool DCArrayParameter::
pack_default_value(DCPackData &pack_data, bool &pack_error) const {
  // We only want to call up if the DCField can pack the value immediately--we
  // don't trust the DCField to generate the default value (since it doesn't
  // know how large the minimum length array is).
  if (_has_default_value && !_default_value_stale) {
    return DCField::pack_default_value(pack_data, pack_error);
  }

  // If a default value is not specified for a variable-length array, the
  // default is the minimum array.
  unsigned int minimum_length = 0;
  if (!_array_size_range.is_empty()) {
    minimum_length = _array_size_range.get_min(0);
  }

  DCPacker packer;
  packer.begin_pack(this);
  packer.push();
  for (unsigned int i = 0; i < minimum_length; i++) {
    packer.pack_default_value();
  }
  packer.pop();
  if (!packer.end_pack()) {
    pack_error = true;
  } else {
    pack_data.append_data(packer.get_data(), packer.get_length());
  }

  return true;
}

/**
 * Unpacks the current numeric or string value from the stream.
 */
void DCArrayParameter::
unpack_string(const char *data, size_t length, size_t &p, string &value,
              bool &pack_error, bool &range_error) const {
  // We can only unpack a string if the array element type is char or int8.
  DCSimpleParameter *simple_type = _element_type->as_simple_parameter();
  if (simple_type == nullptr) {
    pack_error = true;
    return;
  }

  size_t string_length;

  switch (simple_type->get_type()) {
  case ST_char:
  case ST_uint8:
  case ST_int8:
    if (_num_length_bytes != 0) {
      string_length = do_unpack_uint16(data + p);
      p += 2;
    } else {
      nassertv(_array_size >= 0);
      string_length = _array_size;
    }
    if (p + string_length > length) {
      pack_error = true;
      return;
    }
    value.assign(data + p, string_length);
    p += string_length;
    break;

  default:
    pack_error = true;
  }
}

/**
 * Unpacks the current numeric or string value from the stream.
 */
void DCArrayParameter::
unpack_blob(const char *data, size_t length, size_t &p, vector_uchar &value,
            bool &pack_error, bool &range_error) const {
  // We can only unpack a string if the array element type is char or int8.
  DCSimpleParameter *simple_type = _element_type->as_simple_parameter();
  if (simple_type == nullptr) {
    pack_error = true;
    return;
  }

  size_t blob_size;

  switch (simple_type->get_type()) {
  case ST_char:
  case ST_uint8:
  case ST_int8:
    if (_num_length_bytes != 0) {
      blob_size = do_unpack_uint16(data + p);
      p += 2;
    } else {
      nassertv(_array_size >= 0);
      blob_size = _array_size;
    }
    if (p + blob_size > length) {
      pack_error = true;
      return;
    }
    value = vector_uchar((const unsigned char *)data + p,
                         (const unsigned char *)data + p + blob_size);
    p += blob_size;
    break;

  default:
    pack_error = true;
  }
}

/**
 * Returns true if the other interface is bitwise the same as this one--that
 * is, a uint32 only matches a uint32, etc.  Names of components, and range
 * limits, are not compared.
 */
bool DCArrayParameter::
do_check_match(const DCPackerInterface *other) const {
  return other->do_check_match_array_parameter(this);
}

/**
 * Returns true if this field matches the indicated simple parameter, false
 * otherwise.
 */
bool DCArrayParameter::
do_check_match_simple_parameter(const DCSimpleParameter *other) const {
  return ((const DCPackerInterface *)other)->do_check_match_array_parameter(this);
}

/**
 * Returns true if this field matches the indicated class parameter, false
 * otherwise.
 */
bool DCArrayParameter::
do_check_match_class_parameter(const DCClassParameter *other) const {
  return ((const DCPackerInterface *)other)->do_check_match_array_parameter(this);
}

/**
 * Returns true if this field matches the indicated array parameter, false
 * otherwise.
 */
bool DCArrayParameter::
do_check_match_array_parameter(const DCArrayParameter *other) const {
  if (_array_size != other->_array_size) {
    return false;
  }
  return _element_type->check_match(other->_element_type);
}
