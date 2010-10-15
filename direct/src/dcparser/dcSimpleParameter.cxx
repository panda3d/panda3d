// Filename: dcSimpleParameter.cxx
// Created by:  drose (15Jun04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "dcSimpleParameter.h"
#include "dcPackData.h"
#include "dcTypedef.h"
#include "dcArrayParameter.h"
#include "dcClassParameter.h"
#include "dcClass.h"
#include "hashGenerator.h"
#include <math.h>

DCSimpleParameter::NestedFieldMap DCSimpleParameter::_nested_field_map;
DCClassParameter *DCSimpleParameter::_uint32uint8_type = NULL;

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
DCSimpleParameter::
DCSimpleParameter(DCSubatomicType type, unsigned int divisor) :
  _type(type),
  _divisor(1),
  _has_modulus(false)
{
  _pack_type = PT_invalid;
  _nested_type = ST_invalid;
  _has_nested_fields = false;
  _bytes_per_element = 0;
  _num_length_bytes = 2;

  // Check for one of the built-in array types.  For these types, we
  // must present a packing interface that has a variable number of
  // nested fields of the appropriate type.
  switch (_type) {
  case ST_int8array:
    _pack_type = PT_array;
    _nested_type = ST_int8;
    _has_nested_fields = true;
    _bytes_per_element = 1;
    break;

  case ST_int16array:
    _pack_type = PT_array;
    _nested_type = ST_int16;
    _has_nested_fields = true;
    _bytes_per_element = 2;
    break;

  case ST_int32array:
    _pack_type = PT_array;
    _nested_type = ST_int32;
    _has_nested_fields = true;
    _bytes_per_element = 4;
    break;

  case ST_uint8array:
    _pack_type = PT_array;
    _nested_type = ST_uint8;
    _has_nested_fields = true;
    _bytes_per_element = 1;
    break;

  case ST_uint16array:
    _pack_type = PT_array;
    _nested_type = ST_uint16;
    _has_nested_fields = true;
    _bytes_per_element = 2;
    break;

  case ST_uint32array:
    _pack_type = PT_array;
    _nested_type = ST_uint32;
    _has_nested_fields = true;
    _bytes_per_element = 4;
    break;

  case ST_uint32uint8array:
    _pack_type = PT_array;
    _has_nested_fields = true;
    _bytes_per_element = 5;
    break;

  case ST_blob32:
    _num_length_bytes = 4;
    // fall through
  case ST_blob:
    // For blob and string, we will present an array interface
    // as an array of uint8, but we will also accept a set_value()
    // with a string parameter.
    _pack_type = PT_blob;
    _nested_type = ST_uint8;
    _has_nested_fields = true;
    _bytes_per_element = 1;
    break;

  case ST_string: 
    _pack_type = PT_string;
    _nested_type = ST_char;
    _has_nested_fields = true;
    _bytes_per_element = 1;
    break;

    // The simple types can be packed directly.
  case ST_int8:
    _pack_type = PT_int;
    _has_fixed_byte_size = true;
    _fixed_byte_size = 1;
    break;

  case ST_int16:
    _pack_type = PT_int;
    _has_fixed_byte_size = true;
    _fixed_byte_size = 2;
    break;

  case ST_int32:
    _pack_type = PT_int;
    _has_fixed_byte_size = true;
    _fixed_byte_size = 4;
    break;

  case ST_int64:
    _pack_type = PT_int64;
    _has_fixed_byte_size = true;
    _fixed_byte_size = 8;
    break;

  case ST_char:
    _pack_type = PT_string;
    _has_fixed_byte_size = true;
    _fixed_byte_size = 1;
    break;

  case ST_uint8:
    _pack_type = PT_uint;
    _has_fixed_byte_size = true;
    _fixed_byte_size = 1;
    break;

  case ST_uint16:
    _pack_type = PT_uint;
    _has_fixed_byte_size = true;
    _fixed_byte_size = 2;
    break;

  case ST_uint32:
    _pack_type = PT_uint;
    _has_fixed_byte_size = true;
    _fixed_byte_size = 4;
    break;

  case ST_uint64:
    _pack_type = PT_uint64;
    _has_fixed_byte_size = true;
    _fixed_byte_size = 8;
    break;

  case ST_float64:
    _pack_type = PT_double;
    _has_fixed_byte_size = true;
    _fixed_byte_size = 8;
    break;

  case ST_invalid:
    break;
  }
  _has_fixed_structure = _has_fixed_byte_size;

  set_divisor(divisor);

  if (_nested_type != ST_invalid) {
    _nested_field = create_nested_field(_nested_type, _divisor);

  } else if (_type == ST_uint32uint8array) {
    // This one is a special case.  We must create a special nested
    // type that accepts a uint32 followed by a uint8 for each
    // element.
    _nested_field = create_uint32uint8_type();

  } else {
    _nested_field = NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
DCSimpleParameter::
DCSimpleParameter(const DCSimpleParameter &copy) :
  DCParameter(copy),
  _type(copy._type),
  _divisor(copy._divisor),
  _nested_field(copy._nested_field),
  _bytes_per_element(copy._bytes_per_element),
  _orig_range(copy._orig_range),
  _has_modulus(copy._has_modulus),
  _orig_modulus(copy._orig_modulus),
  _int_range(copy._int_range),
  _uint_range(copy._uint_range),
  _int64_range(copy._int64_range),
  _uint64_range(copy._uint64_range),
  _double_range(copy._double_range),
  _uint_modulus(copy._uint_modulus),
  _uint64_modulus(copy._uint64_modulus),
  _double_modulus(copy._double_modulus)
{
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::as_simple_parameter
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
DCSimpleParameter *DCSimpleParameter::
as_simple_parameter() {
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::as_simple_parameter
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
const DCSimpleParameter *DCSimpleParameter::
as_simple_parameter() const {
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::make_copy
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
DCParameter *DCSimpleParameter::
make_copy() const {
  return new DCSimpleParameter(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::is_valid
//       Access: Published, Virtual
//  Description: Returns false if the type is an invalid type
//               (e.g. declared from an undefined typedef), true if
//               it is valid.
////////////////////////////////////////////////////////////////////
bool DCSimpleParameter::
is_valid() const {
  return _type != ST_invalid;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::get_type
//       Access: Published
//  Description: Returns the particular subatomic type represented by
//               this instance.
////////////////////////////////////////////////////////////////////
DCSubatomicType DCSimpleParameter::
get_type() const {
  return _type;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::has_modulus
//       Access: Published
//  Description: Returns true if there is a modulus associated, false
//               otherwise.,
////////////////////////////////////////////////////////////////////
bool DCSimpleParameter::
has_modulus() const {
  return _has_modulus;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::get_modulus
//       Access: Published
//  Description: Returns the modulus associated with this type, if
//               any.  It is an error to call this if has_modulus()
//               returned false.
//
//               If present, this is the modulus that is used to
//               constrain the numeric value of the field before it is
//               packed (and range-checked).
////////////////////////////////////////////////////////////////////
double DCSimpleParameter::
get_modulus() const {
  return _orig_modulus;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::get_divisor
//       Access: Published
//  Description: Returns the divisor associated with this type.  This
//               is 1 by default, but if this is other than one it
//               represents the scale to apply when packing and
//               unpacking numeric values (to store fixed-point values
//               in an integer field).  It is only meaningful for
//               numeric-type fields.
////////////////////////////////////////////////////////////////////
int DCSimpleParameter::
get_divisor() const {
  return _divisor;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::is_numeric_type
//       Access: Public
//  Description: Returns true if the type is a numeric type (and
//               therefore can accept a divisor and/or a modulus), or
//               false if it is some string-based type.
////////////////////////////////////////////////////////////////////
bool DCSimpleParameter::
is_numeric_type() const {
  return !(_pack_type == PT_string || _pack_type == PT_blob);
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::set_modulus
//       Access: Public
//  Description: Assigns the indicated modulus to the simple type.
//               Any packed value will be constrained to be within [0,
//               modulus).
//
//               Returns true if assigned, false if this type cannot
//               accept a modulus or if the modulus is invalid.
////////////////////////////////////////////////////////////////////
bool DCSimpleParameter::
set_modulus(double modulus) {
  if (_pack_type == PT_string || _pack_type == PT_blob || modulus <= 0.0) {
    return false;
  }

  _has_modulus = true;
  _orig_modulus = modulus;

  bool range_error = false;
  _double_modulus = modulus * _divisor;
  _uint64_modulus = (PN_uint64)floor(_double_modulus + 0.5);
  _uint_modulus = (unsigned int)_uint64_modulus;

  // Check the range.  The legitimate range for a modulus value is 1
  // through (maximum_value + 1).
  switch (_type) {
  case ST_int8:
  case ST_int8array:
    validate_uint64_limits(_uint64_modulus - 1, 7, range_error);
    break;

  case ST_int16:
  case ST_int16array:
    validate_uint64_limits(_uint64_modulus - 1, 15, range_error);
    break;

  case ST_int32:
  case ST_int32array:
    validate_uint64_limits(_uint64_modulus - 1, 31, range_error);
    break;
    
  case ST_int64:
    validate_uint64_limits(_uint64_modulus - 1, 63, range_error);
    break;

  case ST_char:
  case ST_uint8:
  case ST_uint8array:
    validate_uint64_limits(_uint64_modulus - 1, 8, range_error);
    break;

  case ST_uint16:
  case ST_uint16array:
    validate_uint64_limits(_uint64_modulus - 1, 16, range_error);
    break;

  case ST_uint32:
  case ST_uint32array:
    validate_uint64_limits(_uint64_modulus - 1, 32, range_error);
    break;
    
  case ST_uint64:
  case ST_float64:
    break;

  default:
    return false;
  }

  return !range_error;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::set_divisor
//       Access: Public
//  Description: Assigns the indicated divisor to the simple type.
//               Returns true if assigned, false if this type cannot
//               accept a divisor or if the divisor is invalid.
////////////////////////////////////////////////////////////////////
bool DCSimpleParameter::
set_divisor(unsigned int divisor) {
  if (_pack_type == PT_string || _pack_type == PT_blob || divisor == 0) {
    return false;
  }

  _divisor = divisor;
  if ((_divisor != 1) &&
      (_pack_type == PT_int || _pack_type == PT_int64 ||
       _pack_type == PT_uint || _pack_type == PT_uint64)) {
    _pack_type = PT_double;
  }

  if (_has_range_limits) {
    set_range(_orig_range);
  }
  if (_has_modulus) {
    set_modulus(_orig_modulus);
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::set_range
//       Access: Public
//  Description: Sets the parameter with the indicated range.  A
//               DCDoubleRange is used for specification, since this
//               is the most generic type; but it is converted to the
//               appropriate type internally.  The return value is
//               true if successful, or false if the range is
//               inappropriate for the type.
////////////////////////////////////////////////////////////////////
bool DCSimpleParameter::
set_range(const DCDoubleRange &range) {
  bool range_error = false;
  int num_ranges = range.get_num_ranges();
  int i;

  _has_range_limits = (num_ranges != 0);
  _orig_range = range;

  switch (_type) {
  case ST_int8:
  case ST_int8array:
    _int_range.clear();
    for (i = 0; i < num_ranges; i++) {
      PN_int64 min = (PN_int64)floor(range.get_min(i) * _divisor + 0.5);
      PN_int64 max = (PN_int64)floor(range.get_max(i) * _divisor + 0.5);
      validate_int64_limits(min, 8, range_error);
      validate_int64_limits(max, 8, range_error);
      _int_range.add_range((int)min, (int)max);
    }
    break;
    
  case ST_int16:
  case ST_int16array:
    _int_range.clear();
    for (i = 0; i < num_ranges; i++) {
      PN_int64 min = (PN_int64)floor(range.get_min(i) * _divisor + 0.5);
      PN_int64 max = (PN_int64)floor(range.get_max(i) * _divisor + 0.5);
      validate_int64_limits(min, 16, range_error);
      validate_int64_limits(max, 16, range_error);
      _int_range.add_range((int)min, (int)max);
    }
    break;
    
  case ST_int32:
  case ST_int32array:
    _int_range.clear();
    for (i = 0; i < num_ranges; i++) {
      PN_int64 min = (PN_int64)floor(range.get_min(i) * _divisor + 0.5);
      PN_int64 max = (PN_int64)floor(range.get_max(i) * _divisor + 0.5);
      validate_int64_limits(min, 32, range_error);
      validate_int64_limits(max, 32, range_error);
      _int_range.add_range((int)min, (int)max);
    }
    break;
    
  case ST_int64:
    _int64_range.clear();
    for (i = 0; i < num_ranges; i++) {
      PN_int64 min = (PN_int64)floor(range.get_min(i) * _divisor + 0.5);
      PN_int64 max = (PN_int64)floor(range.get_max(i) * _divisor + 0.5);
      _int64_range.add_range(min, max);
    }
    break;
    
  case ST_char:
  case ST_uint8:
  case ST_uint8array:
    _uint_range.clear();
    for (i = 0; i < num_ranges; i++) {
      PN_uint64 min = (PN_uint64)floor(range.get_min(i) * _divisor + 0.5);
      PN_uint64 max = (PN_uint64)floor(range.get_max(i) * _divisor + 0.5);
      validate_uint64_limits(min, 8, range_error);
      validate_uint64_limits(max, 8, range_error);
      _uint_range.add_range((unsigned int)min, (unsigned int)max);
    }
    break;
    
  case ST_uint16:
  case ST_uint16array:
    _uint_range.clear();
    for (i = 0; i < num_ranges; i++) {
      PN_uint64 min = (PN_uint64)floor(range.get_min(i) * _divisor + 0.5);
      PN_uint64 max = (PN_uint64)floor(range.get_max(i) * _divisor + 0.5);
      validate_uint64_limits(min, 16, range_error);
      validate_uint64_limits(max, 16, range_error);
      _uint_range.add_range((unsigned int)min, (unsigned int)max);
    }
    break;
    
  case ST_uint32:
  case ST_uint32array:
    _uint_range.clear();
    for (i = 0; i < num_ranges; i++) {
      PN_uint64 min = (PN_uint64)floor(range.get_min(i) * _divisor + 0.5);
      PN_uint64 max = (PN_uint64)floor(range.get_max(i) * _divisor + 0.5);
      validate_uint64_limits(min, 32, range_error);
      validate_uint64_limits(max, 32, range_error);
      _uint_range.add_range((unsigned int)min, (unsigned int)max);
    }
    break;
    
  case ST_uint64:
    _uint64_range.clear();
    for (i = 0; i < num_ranges; i++) {
      PN_uint64 min = (PN_uint64)floor(range.get_min(i) * _divisor + 0.5);
      PN_uint64 max = (PN_uint64)floor(range.get_max(i) * _divisor + 0.5);
      _uint64_range.add_range(min, max);
    }
    break;

  case ST_float64:
    _double_range.clear();
    for (i = 0; i < num_ranges; i++) {
      double min = range.get_min(i) * _divisor;
      double max = range.get_max(i) * _divisor;
      _double_range.add_range(min, max);
    }
    break;

  case ST_string:
  case ST_blob:
    _uint_range.clear();
    for (i = 0; i < num_ranges; i++) {
      PN_uint64 min = (PN_uint64)floor(range.get_min(i) * _divisor + 0.5);
      PN_uint64 max = (PN_uint64)floor(range.get_max(i) * _divisor + 0.5);
      validate_uint64_limits(min, 16, range_error);
      validate_uint64_limits(max, 16, range_error);
      _uint_range.add_range((unsigned int)min, (unsigned int)max);
    }
    if (_uint_range.has_one_value()) {
      // If we now have a fixed-length string requirement, we don't
      // need a leading number of bytes.
      _num_length_bytes = 0;
      _has_fixed_byte_size = true;
      _fixed_byte_size = _uint_range.get_one_value();
      _has_fixed_structure = true;
    } else {
      _num_length_bytes = 2;
      _has_fixed_byte_size = false;
      _has_fixed_structure = false;
    }
    break;

  case ST_blob32:
    _uint_range.clear();
    for (i = 0; i < num_ranges; i++) {
      PN_uint64 min = (PN_uint64)floor(range.get_min(i) * _divisor + 0.5);
      PN_uint64 max = (PN_uint64)floor(range.get_max(i) * _divisor + 0.5);
      validate_uint64_limits(min, 32, range_error);
      validate_uint64_limits(max, 32, range_error);
      _uint_range.add_range((unsigned int)min, (unsigned int)max);
    }
    if (_uint_range.has_one_value()) {
      // If we now have a fixed-length string requirement, we don't
      // need a leading number of bytes.
      _num_length_bytes = 0;
      _has_fixed_byte_size = true;
      _fixed_byte_size = _uint_range.get_one_value();
      _has_fixed_structure = true;
    } else {
      _num_length_bytes = 4;
      _has_fixed_byte_size = false;
      _has_fixed_structure = false;
    }
    break;

  default:
    return false;
  }

  return !range_error;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::calc_num_nested_fields
//       Access: Public, Virtual
//  Description: This flavor of get_num_nested_fields is used during
//               unpacking.  It returns the number of nested fields to
//               expect, given a certain length in bytes (as read from
//               the _num_length_bytes stored in the stream on the
//               push).  This will only be called if _num_length_bytes
//               is nonzero.
////////////////////////////////////////////////////////////////////
int DCSimpleParameter::
calc_num_nested_fields(size_t length_bytes) const {
  if (_bytes_per_element != 0) {
    return length_bytes / _bytes_per_element;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::get_nested_field
//       Access: Public, Virtual
//  Description: Returns the DCPackerInterface object that represents
//               the nth nested field.  This may return NULL if there
//               is no such field (but it shouldn't do this if n is in
//               the range 0 <= n < get_num_nested_fields()).
////////////////////////////////////////////////////////////////////
DCPackerInterface *DCSimpleParameter::
get_nested_field(int) const {
  return _nested_field;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::pack_double
//       Access: Published, Virtual
//  Description: Packs the indicated numeric or string value into the
//               stream.
////////////////////////////////////////////////////////////////////
void DCSimpleParameter::
pack_double(DCPackData &pack_data, double value,
            bool &pack_error, bool &range_error) const {
  double real_value = value * _divisor;
  if (_has_modulus) {
    if (real_value < 0.0) {
      real_value = _double_modulus - fmod(-real_value, _double_modulus);
      if (real_value == _double_modulus) {
        real_value = 0.0;
      }
    } else {
      real_value = fmod(real_value, _double_modulus);
    }
  }

  switch (_type) {
  case ST_int8:
    {
      int int_value = (int)floor(real_value + 0.5);
      _int_range.validate(int_value, range_error);
      validate_int_limits(int_value, 8, range_error);
      do_pack_int8(pack_data.get_write_pointer(1), int_value);
    }
    break;

  case ST_int16:
    {
      int int_value = (int)floor(real_value + 0.5);
      _int_range.validate(int_value, range_error);
      validate_int_limits(int_value, 16, range_error);
      do_pack_int16(pack_data.get_write_pointer(2), int_value);
    }
    break;
    
  case ST_int32:
    {
      int int_value = (int)floor(real_value + 0.5);
      _int_range.validate(int_value, range_error);
      do_pack_int32(pack_data.get_write_pointer(4), int_value);
    }
    break;
    
  case ST_int64:
    {
      PN_int64 int64_value = (PN_int64)floor(real_value + 0.5);
      _int64_range.validate(int64_value, range_error);
      do_pack_int64(pack_data.get_write_pointer(8), int64_value);
    }
    break;
    
  case ST_char:
  case ST_uint8:
    {
      unsigned int int_value = (unsigned int)floor(real_value + 0.5);
      _uint_range.validate(int_value, range_error);
      validate_uint_limits(int_value, 8, range_error);
      do_pack_uint8(pack_data.get_write_pointer(1), int_value);
    }
    break;
    
  case ST_uint16:
    {
      unsigned int int_value = (unsigned int)floor(real_value + 0.5);
      _uint_range.validate(int_value, range_error);
      validate_uint_limits(int_value, 16, range_error);
      do_pack_uint16(pack_data.get_write_pointer(2), int_value);
    }
    break;
    
  case ST_uint32:
    {
      unsigned int int_value = (unsigned int)floor(real_value + 0.5);
      _uint_range.validate(int_value, range_error);
      do_pack_uint32(pack_data.get_write_pointer(4), int_value);
    }
    break;
    
  case ST_uint64:
    {
      PN_uint64 int64_value = (PN_uint64)floor(real_value + 0.5);
      _uint64_range.validate(int64_value, range_error);
      do_pack_uint64(pack_data.get_write_pointer(8), int64_value);
    }
    break;

  case ST_float64:
    _double_range.validate(real_value, range_error);
    do_pack_float64(pack_data.get_write_pointer(8), real_value);
    break;

  default:
    pack_error = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::pack_int
//       Access: Published, Virtual
//  Description: Packs the indicated numeric or string value into the
//               stream.
////////////////////////////////////////////////////////////////////
void DCSimpleParameter::
pack_int(DCPackData &pack_data, int value,
         bool &pack_error, bool &range_error) const {
  int int_value = value * _divisor;

  if (value != 0 && (int_value / value) != (int)_divisor) {
    // If we've experienced overflow after applying the divisor, pack
    // it as an int64 instead.
    pack_int64(pack_data, (PN_int64)value, pack_error, range_error);
    return;
  }

  if (_has_modulus && _uint_modulus != 0) {
    if (int_value < 0) {
      int_value = _uint_modulus - 1 - (-int_value - 1) % _uint_modulus;
    } else {
      int_value = int_value % _uint_modulus;
    }
  }

  switch (_type) {
  case ST_int8:
    _int_range.validate(int_value, range_error);
    validate_int_limits(int_value, 8, range_error);
    do_pack_int8(pack_data.get_write_pointer(1), int_value);
    break;

  case ST_int16:
    _int_range.validate(int_value, range_error);
    validate_int_limits(int_value, 16, range_error);
    do_pack_int16(pack_data.get_write_pointer(2), int_value);
    break;

  case ST_int32:
    _int_range.validate(int_value, range_error);
    do_pack_int32(pack_data.get_write_pointer(4), int_value);
    break;

  case ST_int64:
    _int64_range.validate(int_value, range_error);
    do_pack_int64(pack_data.get_write_pointer(8), int_value);
    break;

  case ST_char:
  case ST_uint8:
    if (int_value < 0) {
      range_error = true;
    }
    _uint_range.validate((unsigned int)int_value, range_error);
    validate_uint_limits((unsigned int)int_value, 8, range_error);
    do_pack_uint8(pack_data.get_write_pointer(1), (unsigned int)int_value);
    break;

  case ST_uint16:
    if (int_value < 0) {
      range_error = true;
    }
    _uint_range.validate((unsigned int)int_value, range_error);
    validate_uint_limits((unsigned int)int_value, 16, range_error);
    do_pack_uint16(pack_data.get_write_pointer(2), (unsigned int)int_value);
    break;

  case ST_uint32:
    if (int_value < 0) {
      range_error = true;
    }
    _uint_range.validate((unsigned int)int_value, range_error);
    do_pack_uint32(pack_data.get_write_pointer(4), (unsigned int)int_value);
    break;

  case ST_uint64:
    if (int_value < 0) {
      range_error = true;
    }
    _uint64_range.validate((unsigned int)int_value, range_error);
    do_pack_uint64(pack_data.get_write_pointer(8), (unsigned int)int_value);
    break;

  case ST_float64:
    _double_range.validate(int_value, range_error);
    do_pack_float64(pack_data.get_write_pointer(8), int_value);
    break;

  default:
    pack_error = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::pack_uint
//       Access: Published, Virtual
//  Description: Packs the indicated numeric or string value into the
//               stream.
////////////////////////////////////////////////////////////////////
void DCSimpleParameter::
pack_uint(DCPackData &pack_data, unsigned int value,
          bool &pack_error, bool &range_error) const {
  unsigned int int_value = value * _divisor;
  if (_has_modulus && _uint_modulus != 0) {
    int_value = int_value % _uint_modulus;
  }

  switch (_type) {
  case ST_int8:
    if ((int)int_value < 0) {
      range_error = true;
    }
    _int_range.validate((int)int_value, range_error);
    validate_int_limits((int)int_value, 8, range_error);
    do_pack_int8(pack_data.get_write_pointer(1), (int)int_value);
    break;

  case ST_int16:
    if ((int)int_value < 0) {
      range_error = true;
    }
    _int_range.validate((int)int_value, range_error);
    validate_int_limits((int)int_value, 16, range_error);
    do_pack_int16(pack_data.get_write_pointer(2), (int)int_value);
    break;

  case ST_int32:
    if ((int)int_value < 0) {
      range_error = true;
    }
    _int_range.validate((int)int_value, range_error);
    do_pack_int32(pack_data.get_write_pointer(4), (int)int_value);
    break;

  case ST_int64:
    if ((int)int_value < 0) {
      range_error = true;
    }
    _int64_range.validate((int)int_value, range_error);
    do_pack_int64(pack_data.get_write_pointer(8), (int)int_value);
    break;

  case ST_char:
  case ST_uint8:
    _uint_range.validate(int_value, range_error);
    validate_uint_limits(int_value, 8, range_error);
    do_pack_uint8(pack_data.get_write_pointer(1), int_value);
    break;

  case ST_uint16:
    _uint_range.validate(int_value, range_error);
    validate_uint_limits(int_value, 16, range_error);
    do_pack_uint16(pack_data.get_write_pointer(2), int_value);
    break;

  case ST_uint32:
    _uint_range.validate(int_value, range_error);
    do_pack_uint32(pack_data.get_write_pointer(4), int_value);
    break;

  case ST_uint64:
    _uint64_range.validate(int_value, range_error);
    do_pack_uint64(pack_data.get_write_pointer(8), int_value);
    break;

  case ST_float64:
    _double_range.validate(int_value, range_error);
    do_pack_float64(pack_data.get_write_pointer(8), int_value);
    break;

  default:
    pack_error = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::pack_int64
//       Access: Published, Virtual
//  Description: Packs the indicated numeric or string value into the
//               stream.
////////////////////////////////////////////////////////////////////
void DCSimpleParameter::
pack_int64(DCPackData &pack_data, PN_int64 value,
            bool &pack_error, bool &range_error) const {
  PN_int64 int_value = value * _divisor;
  if (_has_modulus && _uint64_modulus != 0) {
    if (int_value < 0) {
      int_value = _uint64_modulus - 1 - (-int_value - 1) % _uint64_modulus;
    } else {
      int_value = int_value % _uint64_modulus;
    }
  }

  switch (_type) {
  case ST_int8:
    _int_range.validate((int)int_value, range_error);
    validate_int64_limits(int_value, 8, range_error);
    do_pack_int8(pack_data.get_write_pointer(1), (int)int_value);
    break;

  case ST_int16:
    _int_range.validate((int)int_value, range_error);
    validate_int64_limits(int_value, 16, range_error);
    do_pack_int16(pack_data.get_write_pointer(2), (int)int_value);
    break;

  case ST_int32:
    _int_range.validate((int)int_value, range_error);
    validate_int64_limits(int_value, 32, range_error);
    do_pack_int32(pack_data.get_write_pointer(4), (int)int_value);
    break;

  case ST_int64:
    _int64_range.validate(int_value, range_error);
    do_pack_int64(pack_data.get_write_pointer(8), int_value);
    break;

  case ST_char:
  case ST_uint8:
    if (int_value < 0) {
      range_error = true;
    }
    _uint_range.validate((unsigned int)(PN_uint64)int_value, range_error);
    validate_uint64_limits((PN_uint64)int_value, 8, range_error);
    do_pack_uint8(pack_data.get_write_pointer(1), (unsigned int)(PN_uint64)int_value);
    break;

  case ST_uint16:
    if (int_value < 0) {
      range_error = true;
    }
    _uint_range.validate((unsigned int)(PN_uint64)int_value, range_error);
    validate_uint64_limits((PN_uint64)int_value, 16, range_error);
    do_pack_uint16(pack_data.get_write_pointer(2), (unsigned int)(PN_uint64)int_value);
    break;

  case ST_uint32:
    if (int_value < 0) {
      range_error = true;
    }
    _uint_range.validate((unsigned int)(PN_uint64)int_value, range_error);
    validate_uint64_limits((PN_uint64)int_value, 32, range_error);
    do_pack_uint32(pack_data.get_write_pointer(4), (unsigned int)(PN_uint64)int_value);
    break;

  case ST_uint64:
    if (int_value < 0) {
      range_error = true;
    }
    _uint64_range.validate((PN_uint64)int_value, range_error);
    do_pack_uint64(pack_data.get_write_pointer(8), (PN_uint64)int_value);
    break;

  case ST_float64:
    _double_range.validate((double)int_value, range_error);
    do_pack_float64(pack_data.get_write_pointer(8), (double)int_value);
    break;

  default:
    pack_error = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::pack_uint64
//       Access: Published, Virtual
//  Description: Packs the indicated numeric or string value into the
//               stream.
////////////////////////////////////////////////////////////////////
void DCSimpleParameter::
pack_uint64(DCPackData &pack_data, PN_uint64 value,
            bool &pack_error, bool &range_error) const {
  PN_uint64 int_value = value * _divisor;
  if (_has_modulus && _uint64_modulus != 0) {
    int_value = int_value % _uint64_modulus;
  }

  switch (_type) {
  case ST_int8:
    if ((PN_int64)int_value < 0) {
      range_error = true;
    }
    _int_range.validate((int)(PN_int64)int_value, range_error);
    validate_int64_limits((PN_int64)int_value, 8, range_error);
    do_pack_int8(pack_data.get_write_pointer(1), (int)(PN_int64)int_value);
    break;

  case ST_int16:
    if ((PN_int64)int_value < 0) {
      range_error = true;
    }
    _int_range.validate((int)(PN_int64)int_value, range_error);
    validate_int64_limits((PN_int64)int_value, 16, range_error);
    do_pack_int16(pack_data.get_write_pointer(2), (int)(PN_int64)int_value);
    break;

  case ST_int32:
    if ((PN_int64)int_value < 0) {
      range_error = true;
    }
    _int_range.validate((int)(PN_int64)int_value, range_error);
    validate_int64_limits((PN_int64)int_value, 32, range_error);
    do_pack_int32(pack_data.get_write_pointer(4), (int)(PN_int64)int_value);
    break;

  case ST_int64:
    if ((PN_int64)int_value < 0) {
      range_error = true;
    }
    _int64_range.validate((PN_int64)int_value, range_error);
    do_pack_int64(pack_data.get_write_pointer(8), (PN_int64)int_value);
    break;

  case ST_char:
  case ST_uint8:
    _uint_range.validate((unsigned int)int_value, range_error);
    validate_uint64_limits(int_value, 8, range_error);
    do_pack_uint8(pack_data.get_write_pointer(1), (unsigned int)int_value);
    break;

  case ST_uint16:
    _uint_range.validate((unsigned int)int_value, range_error);
    validate_uint64_limits(int_value, 16, range_error);
    do_pack_uint16(pack_data.get_write_pointer(2), (unsigned int)int_value);
    break;

  case ST_uint32:
    _uint_range.validate((unsigned int)int_value, range_error);
    validate_uint64_limits(int_value, 32, range_error);
    do_pack_uint32(pack_data.get_write_pointer(4), (unsigned int)int_value);
    break;

  case ST_uint64:
    _uint64_range.validate(int_value, range_error);
    do_pack_uint64(pack_data.get_write_pointer(8), int_value);
    break;

  case ST_float64:
    _double_range.validate((double)int_value, range_error);
    do_pack_float64(pack_data.get_write_pointer(8), (double)int_value);
    break;

  default:
    pack_error = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::pack_string
//       Access: Published, Virtual
//  Description: Packs the indicated numeric or string value into the
//               stream.
////////////////////////////////////////////////////////////////////
void DCSimpleParameter::
pack_string(DCPackData &pack_data, const string &value,
            bool &pack_error, bool &range_error) const {
  size_t string_length = value.length();

  switch (_type) {
  case ST_char:
  case ST_uint8:
  case ST_int8:
    if (string_length == 0) {
      pack_error = true;
    } else {
      if (string_length != 1) {
        range_error = true;
      }
      _uint_range.validate((unsigned int)value[0], range_error);
      do_pack_uint8(pack_data.get_write_pointer(1), (unsigned int)value[0]);
    }
    break;

  case ST_string:
  case ST_blob:
    _uint_range.validate(string_length, range_error);
    validate_uint_limits(string_length, 16, range_error);
    if (_num_length_bytes != 0) {
      do_pack_uint16(pack_data.get_write_pointer(2), string_length);
    }
    pack_data.append_data(value.data(), string_length);
    break;

  case ST_blob32:
    _uint_range.validate(string_length, range_error);
    if (_num_length_bytes != 0) {
      do_pack_uint32(pack_data.get_write_pointer(4), string_length);
    }
    pack_data.append_data(value.data(), string_length);
    break;

  default:
    pack_error = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::pack_default_value
//       Access: Public, Virtual
//  Description: Packs the simpleParameter's specified default value (or a
//               sensible default if no value is specified) into the
//               stream.  Returns true if the default value is packed,
//               false if the simpleParameter doesn't know how to pack its
//               default value.
////////////////////////////////////////////////////////////////////
bool DCSimpleParameter::
pack_default_value(DCPackData &pack_data, bool &pack_error) const {
  if (has_default_value()) {
    return DCField::pack_default_value(pack_data, pack_error);
  }

  if (_has_nested_fields) {
    // If the simple type is an array (or string) type, pack the
    // appropriate length array, with code similar to
    // DCArrayParameter::pack_default_value().

    unsigned int minimum_length = 0;
    if (!_uint_range.is_empty()) {
      minimum_length = _uint_range.get_min(0);
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

  } else {
    // Otherwise, if it's just a simple numeric type, pack a zero or
    // the minimum value.
    switch (_type) {
    case ST_int8:
    case ST_int16:
    case ST_int32:
      if (_int_range.is_in_range(0)) {
        pack_int(pack_data, 0, pack_error, pack_error);
      } else {
        pack_int(pack_data, _int_range.get_min(0), pack_error, pack_error);
      }
      break;

    case ST_int64:
      if (_int64_range.is_in_range(0)) {
        pack_int64(pack_data, 0, pack_error, pack_error);
      } else {
        pack_int64(pack_data, _int64_range.get_min(0), pack_error, pack_error);
      }
      break;

    case ST_char:
    case ST_uint8:
    case ST_uint16:
    case ST_uint32:
      if (_uint_range.is_in_range(0)) {
        pack_uint(pack_data, 0, pack_error, pack_error);
      } else {
        pack_uint(pack_data, _uint_range.get_min(0), pack_error, pack_error);
      }
      break;

    case ST_uint64:
      if (_uint64_range.is_in_range(0)) {
        pack_uint64(pack_data, 0, pack_error, pack_error);
      } else {
        pack_uint64(pack_data, _uint64_range.get_min(0), pack_error, pack_error);
      }
      break;
      
    case ST_float64:
      if (_double_range.is_in_range(0.0)) {
        pack_double(pack_data, 0.0, pack_error, pack_error);
      } else {
        pack_double(pack_data, _double_range.get_min(0), pack_error, pack_error);
      }
      break;
      
    default:
      pack_error = true;
    }
  }
  return true;
}



////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::unpack_double
//       Access: Public, Virtual
//  Description: Unpacks the current numeric or string value from the
//               stream.
////////////////////////////////////////////////////////////////////
void DCSimpleParameter::
unpack_double(const char *data, size_t length, size_t &p, double &value,
              bool &pack_error, bool &range_error) const {
  switch (_type) {
  case ST_int8:
    {
      if (p + 1 > length) {
        pack_error = true;
        return;
      }
      int int_value = do_unpack_int8(data + p);
      _int_range.validate(int_value, range_error);
      value = int_value;
      p++;
    }
    break;

  case ST_int16:
    {
      if (p + 2 > length) {
        pack_error = true;
        return;
      }
      int int_value = do_unpack_int16(data + p);
      _int_range.validate(int_value, range_error);
      value = int_value;
      p += 2;
    }
    break;

  case ST_int32:
    {
      if (p + 4 > length) {
        pack_error = true;
        return;
      }
      int int_value = do_unpack_int32(data + p);
      _int_range.validate(int_value, range_error);
      value = int_value;
      p += 4;
    }
    break;

  case ST_int64:
    {
      if (p + 8 > length) {
        pack_error = true;
        return;
      }
      PN_int64 int_value = do_unpack_int64(data + p);
      _int64_range.validate(int_value, range_error);
      value = (double)int_value;
      p += 8;
    }
    break;

  case ST_char:
  case ST_uint8:
    {
      if (p + 1 > length) {
        pack_error = true;
        return;
      }
      unsigned int uint_value = do_unpack_uint8(data + p);
      _uint_range.validate(uint_value, range_error);
      value = uint_value;
      p++;
    }
    break;

  case ST_uint16:
    {
      if (p + 2 > length) {
        pack_error = true;
        return;
      }
      unsigned int uint_value = do_unpack_uint16(data + p);
      _uint_range.validate(uint_value, range_error);
      value = uint_value;
      p += 2;
    }
    break;

  case ST_uint32:
    {
      if (p + 4 > length) {
        pack_error = true;
        return;
      }
      unsigned int uint_value = do_unpack_uint32(data + p);
      _uint_range.validate(uint_value, range_error);
      value = uint_value;
      p += 4;
    }
    break;

  case ST_uint64:
    {
      if (p + 8 > length) {
        pack_error = true;
        return;
      }
      PN_uint64 uint_value = do_unpack_uint64(data + p);
      _uint64_range.validate(uint_value, range_error);
      value = (double)uint_value;
      p += 8;
    }
    break;

  case ST_float64:
    {
      if (p + 8 > length) {
        pack_error = true;
        return;
      }
      value = do_unpack_float64(data + p);
      _double_range.validate(value, range_error);
      p += 8;
    }
    break;

  default:
    pack_error = true;
    return;
  }

  if (_divisor != 1) {
    value = value / _divisor;
  }

  return;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::unpack_int
//       Access: Public, Virtual
//  Description: Unpacks the current numeric or string value from the
//               stream.
////////////////////////////////////////////////////////////////////
void DCSimpleParameter::
unpack_int(const char *data, size_t length, size_t &p, int &value,
           bool &pack_error, bool &range_error) const {
  switch (_type) {
  case ST_int8:
    if (p + 1 > length) {
      pack_error = true;
      return;
    }
    value = do_unpack_int8(data + p);
    _int_range.validate(value, range_error);
    p++;
    break;

  case ST_int16:
    if (p + 2 > length) {
      pack_error = true;
      return;
    }
    value = do_unpack_int16(data + p);
    _int_range.validate(value, range_error);
    p += 2;
    break;

  case ST_int32:
    if (p + 4 > length) {
      pack_error = true;
      return;
    }
    value = do_unpack_int32(data + p);
    _int_range.validate(value, range_error);
    p += 4;
    break;

  case ST_int64:
    {
      if (p + 8 > length) {
        pack_error = true;
        return;
      }
      PN_int64 int_value = do_unpack_uint64(data + p);
      _int64_range.validate(int_value, range_error);
      value = (int)int_value;
      if (value != int_value) {
        // uint exceeded the storage capacity of a signed int.
        pack_error = true;
      }
      p += 8;
    }
    break;

  case ST_char:
  case ST_uint8:
    {
      if (p + 1 > length) {
        pack_error = true;
        return;
      }
      unsigned int uint_value = do_unpack_uint8(data + p);
      _uint_range.validate(uint_value, range_error);
      value = uint_value;
      p++;
    }
    break;

  case ST_uint16:
    {
      if (p + 2 > length) {
        pack_error = true;
        return;
      }
      unsigned int uint_value = do_unpack_uint16(data + p);
      _uint_range.validate(uint_value, range_error);
      value = (int)uint_value;
      p += 2;
    }
    break;

  case ST_uint32:
    {
      if (p + 4 > length) {
        pack_error = true;
        return;
      }
      unsigned int uint_value = do_unpack_uint32(data + p);
      _uint_range.validate(uint_value, range_error);
      value = (int)uint_value;
      if (value < 0) {
        pack_error = true;
      }
      p += 4;
    }
    break;

  case ST_uint64:
    {
      if (p + 8 > length) {
        pack_error = true;
        return;
      }
      PN_uint64 uint_value = do_unpack_uint64(data + p);
      _uint64_range.validate(uint_value, range_error);
      value = (int)(unsigned int)uint_value;
      if ((unsigned int)value != uint_value || value < 0) {
        pack_error = true;
      }
      p += 8;
    }
    break;

  case ST_float64:
    {
      if (p + 8 > length) {
        pack_error = true;
        return;
      }
      double real_value = do_unpack_float64(data + p);
      _double_range.validate(real_value, range_error);
      value = (int)real_value;
      p += 8;
    }
    break;

  default:
    pack_error = true;
    return;
  }

  if (_divisor != 1) {
    value = value / _divisor;
  }

  return;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::unpack_uint
//       Access: Public, Virtual
//  Description: Unpacks the current numeric or string value from the
//               stream.
////////////////////////////////////////////////////////////////////
void DCSimpleParameter::
unpack_uint(const char *data, size_t length, size_t &p, unsigned int &value,
              bool &pack_error, bool &range_error) const {
  switch (_type) {
  case ST_int8:
    {
      if (p + 1 > length) {
        pack_error = true;
      return;
      }
      int int_value = do_unpack_int8(data + p);
      _int_range.validate(int_value, range_error);
      if (int_value < 0) {
        pack_error = true;
      }
      value = (unsigned int)int_value;
      p++;
    }
    break;

  case ST_int16:
    {
      if (p + 2 > length) {
        pack_error = true;
        return;
      }
      int int_value = do_unpack_int16(data + p);
      _int_range.validate(int_value, range_error);
      if (int_value < 0) {
        pack_error = true;
      }
      value = (unsigned int)int_value;
      p += 2;
    }
    break;

  case ST_int32:
    {
      if (p + 4 > length) {
        pack_error = true;
        return;
      }
      int int_value = do_unpack_int32(data + p);
      _int_range.validate(int_value, range_error);
      if (int_value < 0) {
        pack_error = true;
      }
      value = (unsigned int)int_value;
      p += 4;
    }
    break;

  case ST_int64:
    {
      if (p + 8 > length) {
        pack_error = true;
        return;
      }
      PN_int64 int_value = do_unpack_int64(data + p);
      _int64_range.validate(int_value, range_error);
      if (int_value < 0) {
        pack_error = true;
      }
      value = (unsigned int)(int)int_value;
      if (value != int_value) {
        pack_error = true;
      }
      p += 8;
    }
    break;

  case ST_char:
  case ST_uint8:
    if (p + 1 > length) {
      pack_error = true;
      return;
    }
    value = do_unpack_uint8(data + p);
    _uint_range.validate(value, range_error);
    p++;
    break;

  case ST_uint16:
    if (p + 2 > length) {
      pack_error = true;
      return;
    }
    value = do_unpack_uint16(data + p);
    _uint_range.validate(value, range_error);
    p += 2;
    break;

  case ST_uint32:
    if (p + 4 > length) {
      pack_error = true;
      return;
    }
    value = do_unpack_uint32(data + p);
    _uint_range.validate(value, range_error);
    p += 4;
    break;

  case ST_uint64:
    {
      if (p + 8 > length) {
        pack_error = true;
        return;
      }
      PN_uint64 uint_value = do_unpack_uint64(data + p);
      _uint64_range.validate(uint_value, range_error);
      value = (unsigned int)uint_value;
      if (value != uint_value) {
        pack_error = true;
      }
      p += 8;
    }
    break;

  case ST_float64:
    {
      if (p + 8 > length) {
        pack_error = true;
        return;
      }
      double real_value = do_unpack_float64(data + p);
      _double_range.validate(real_value, range_error);
      value = (unsigned int)real_value;
      p += 8;
    }
    break;

  default:
    pack_error = true;
    return;
  }

  if (_divisor != 1) {
    value = value / _divisor;
  }

  return;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::unpack_int64
//       Access: Public, Virtual
//  Description: Unpacks the current numeric or string value from the
//               stream.
////////////////////////////////////////////////////////////////////
void DCSimpleParameter::
unpack_int64(const char *data, size_t length, size_t &p, PN_int64 &value,
              bool &pack_error, bool &range_error) const {
  switch (_type) {
  case ST_int8:
    {
      if (p + 1 > length) {
        pack_error = true;
        return;
      }
      int int_value = do_unpack_int8(data + p);
      _int_range.validate(int_value, range_error);
      value = (PN_int64)int_value;
      p++;
    }
    break;

  case ST_int16:
    {
      if (p + 2 > length) {
        pack_error = true;
        return;
      }
      int int_value = do_unpack_int16(data + p);
      _int_range.validate(int_value, range_error);
      value = (PN_int64)int_value;
      p += 2;
    }
    break;

  case ST_int32:
    {
      if (p + 4 > length) {
        pack_error = true;
        return;
      }
      int int_value = do_unpack_int32(data + p);
      _int_range.validate(int_value, range_error);
      value = (PN_int64)int_value;
      p += 4;
    }
    break;

  case ST_int64:
    if (p + 8 > length) {
      pack_error = true;
      return;
    }
    value = do_unpack_int64(data + p);
    _int64_range.validate(value, range_error);
    p += 8;
    break;

  case ST_char:
  case ST_uint8:
    {
      if (p + 1 > length) {
        pack_error = true;
        return;
      }
      unsigned int uint_value = do_unpack_uint8(data + p);
      _uint_range.validate(uint_value, range_error);
      value = (PN_int64)(int)uint_value;
      p++;
    }
    break;

  case ST_uint16:
    {
      if (p + 2 > length) {
        pack_error = true;
        return;
      }
      unsigned int uint_value = do_unpack_uint16(data + p);
      _uint_range.validate(uint_value, range_error);
      value = (PN_int64)(int)uint_value;
      p += 2;
    }
    break;

  case ST_uint32:
    {
      if (p + 4 > length) {
        pack_error = true;
        return;
      }
      unsigned int uint_value = do_unpack_uint32(data + p);
      _uint_range.validate(uint_value, range_error);
      value = (PN_int64)(int)uint_value;
      p += 4;
    }
    break;

  case ST_uint64:
    {
      if (p + 8 > length) {
        pack_error = true;
        return;
      }
      PN_uint64 uint_value = do_unpack_uint64(data + p);
      _uint64_range.validate(uint_value, range_error);
      value = (PN_int64)uint_value;
      if (value < 0) {
        pack_error = true;
      }
      p += 8;
    }
    break;

  case ST_float64:
    {
      if (p + 8 > length) {
        pack_error = true;
        return;
      }
      double real_value = do_unpack_float64(data + p);
      _double_range.validate(real_value, range_error);
      value = (PN_int64)real_value;
      p += 8;
    }
    break;

  default:
    pack_error = true;
    return;
  }

  if (_divisor != 1) {
    value = value / _divisor;
  }

  return;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::unpack_uint64
//       Access: Public, Virtual
//  Description: Unpacks the current numeric or string value from the
//               stream.
////////////////////////////////////////////////////////////////////
void DCSimpleParameter::
unpack_uint64(const char *data, size_t length, size_t &p, PN_uint64 &value,
              bool &pack_error, bool &range_error) const {
  switch (_type) {
  case ST_int8:
    {
      if (p + 1 > length) {
        pack_error = true;
        return;
      }
      int int_value = do_unpack_int8(data + p);
      _int_range.validate(int_value, range_error);
      if (int_value < 0) {
        pack_error = true;
      }
      value = (PN_uint64)(unsigned int)int_value;
      p++;
    }
    break;

  case ST_int16:
    {
      if (p + 2 > length) {
        pack_error = true;
        return;
      }
      int int_value = do_unpack_int16(data + p);
      _int_range.validate(int_value, range_error);
      if (int_value < 0) {
        pack_error = true;
      }
      value = (PN_uint64)(unsigned int)int_value;
      p += 2;
    }
    break;

  case ST_int32:
    {
      if (p + 4 > length) {
        pack_error = true;
        return;
      }
      int int_value = do_unpack_int32(data + p);
      _int_range.validate(int_value, range_error);
      if (int_value < 0) {
        pack_error = true;
      }
      value = (PN_uint64)(unsigned int)int_value;
      p += 4;
    }
    break;

  case ST_int64:
    {
      if (p + 8 > length) {
        pack_error = true;
        return;
      }
      PN_int64 int_value = do_unpack_int64(data + p);
      _int64_range.validate(int_value, range_error);
      if (int_value < 0) {
        pack_error = true;
      }
      value = (PN_uint64)int_value;
      p += 8;
    }
    break;

  case ST_char:
  case ST_uint8:
    {
      if (p + 1 > length) {
        pack_error = true;
        return;
      }
      unsigned int uint_value = do_unpack_uint8(data + p);
      _uint_range.validate(uint_value, range_error);
      value = (PN_uint64)uint_value;
      p++;
    }
    break;

  case ST_uint16:
    {
      if (p + 2 > length) {
        pack_error = true;
        return;
      }
      unsigned int uint_value = do_unpack_uint16(data + p);
      _uint_range.validate(uint_value, range_error);
      value = (PN_uint64)uint_value;
      p += 2;
    }
    break;

  case ST_uint32:
    {
      if (p + 4 > length) {
        pack_error = true;
        return;
      }
      unsigned int uint_value = do_unpack_uint32(data + p);
      _uint_range.validate(uint_value, range_error);
      value = (PN_uint64)uint_value;
      p += 4;
    }
    break;

  case ST_uint64:
    if (p + 8 > length) {
      pack_error = true;
      return;
    }
    value = do_unpack_uint64(data + p);
    _uint64_range.validate(value, range_error);
    p += 8;
    break;

  case ST_float64:
    {
      if (p + 8 > length) {
        pack_error = true;
        return;
      }
      double real_value = do_unpack_float64(data + p);
      _double_range.validate(real_value, range_error);
      value = (PN_uint64)real_value;
      p += 8;
    }
    break;

  default:
    pack_error = true;
    return;
  }

  if (_divisor != 1) {
    value = value / _divisor;
  }

  return;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::unpack_string
//       Access: Public, Virtual
//  Description: Unpacks the current numeric or string value from the
//               stream.
////////////////////////////////////////////////////////////////////
void DCSimpleParameter::
unpack_string(const char *data, size_t length, size_t &p, string &value,
              bool &pack_error, bool &range_error) const {
  // If the type is a single byte, unpack it into a string of length 1.
  switch (_type) {
  case ST_char:
  case ST_int8:
  case ST_uint8:
    {
      if (p + 1 > length) {
        pack_error = true;
        return;
      }
      unsigned int int_value = do_unpack_uint8(data + p);
      _uint_range.validate(int_value, range_error);
      value.assign(1, int_value);
      p++;
    }
    return;

  default:
    break;
  }

  size_t string_length;

  if (_num_length_bytes == 0) {
    string_length = _fixed_byte_size;

  } else {
    switch (_type) {
    case ST_string:
    case ST_blob:
      if (p + 2 > length) {
        pack_error = true;
        return;
      }
      string_length = do_unpack_uint16(data + p);
      p += 2;
      break;
      
    case ST_blob32:
      if (p + 4 > length) {
        pack_error = true;
        return;
      }
      string_length = do_unpack_uint32(data + p);
      p += 4;
      break;
      
    default:
      pack_error = true;
      return;
    }
  }

  _uint_range.validate(string_length, range_error);

  if (p + string_length > length) {
    pack_error = true;
    return;
  }
  value.assign(data + p, string_length);
  p += string_length;

  return;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::unpack_validate
//       Access: Public, Virtual
//  Description: Internally unpacks the current numeric or string
//               value and validates it against the type range limits,
//               but does not return the value.  Returns true on
//               success, false on failure (e.g. we don't know how to
//               validate this field).
////////////////////////////////////////////////////////////////////
bool DCSimpleParameter::
unpack_validate(const char *data, size_t length, size_t &p,
                bool &pack_error, bool &range_error) const { 
  if (!_has_range_limits) {
    return unpack_skip(data, length, p, pack_error);
  }
  switch (_type) {
  case ST_int8:
    {
      if (p + 1 > length) {
        pack_error = true;
        return true;
      }
      int int_value = do_unpack_int8(data + p);
      _int_range.validate(int_value, range_error);
      p++;
    }
    break;

  case ST_int16:
    {
      if (p + 2 > length) {
        pack_error = true;
        return true;
      }
      int int_value = do_unpack_int16(data + p);
      _int_range.validate(int_value, range_error);
      p += 2;
    }
    break;

  case ST_int32:
    {
      if (p + 4 > length) {
        pack_error = true;
        return true;
      }
      int int_value = do_unpack_int32(data + p);
      _int_range.validate(int_value, range_error);
      p += 4;
    }
    break;

  case ST_int64:
    {
      if (p + 8 > length) {
        pack_error = true;
        return true;
      }
      PN_int64 int_value = do_unpack_int64(data + p);
      _int64_range.validate(int_value, range_error);
      p += 8;
    }
    break;

  case ST_char:
  case ST_uint8:
    {
      if (p + 1 > length) {
        pack_error = true;
        return true;
      }
      unsigned int uint_value = do_unpack_uint8(data + p);
      _uint_range.validate(uint_value, range_error);
      p++;
    }
    break;

  case ST_uint16:
    {
      if (p + 2 > length) {
        pack_error = true;
        return true;
      }
      unsigned int uint_value = do_unpack_uint16(data + p);
      _uint_range.validate(uint_value, range_error);
      p += 2;
    }
    break;

  case ST_uint32:
    {
      if (p + 4 > length) {
        pack_error = true;
        return true;
      }
      unsigned int uint_value = do_unpack_uint32(data + p);
      _uint_range.validate(uint_value, range_error);
      p += 4;
    }
    break;

  case ST_uint64:
    {
      if (p + 8 > length) {
        pack_error = true;
        return true;
      }
      PN_uint64 uint_value = do_unpack_uint64(data + p);
      _uint64_range.validate(uint_value, range_error);
      p += 8;
    }
    break;

  case ST_float64:
    {
      if (p + 8 > length) {
        pack_error = true;
        return true;
      }
      double real_value = do_unpack_float64(data + p);
      _double_range.validate(real_value, range_error);
      p += 8;
    }
    break;

  case ST_string:
  case ST_blob:
    if (_num_length_bytes == 0) {
      p += _fixed_byte_size;

    } else {
      if (p + 2 > length) {
        pack_error = true;
        return true;
      }
      size_t string_length = do_unpack_uint16(data + p);
      _uint_range.validate(string_length, range_error);
      p += 2 + string_length;
    }
    break;

  case ST_blob32:
    if (_num_length_bytes == 0) {
      p += _fixed_byte_size;

    } else {
      if (p + 4 > length) {
        pack_error = true;
        return true;
      }
      size_t string_length = do_unpack_uint32(data + p);
      _uint_range.validate(string_length, range_error);
      p += 4 + string_length;
    }
    break;

  default:
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::unpack_skip
//       Access: Public, Virtual
//  Description: Increments p to the end of the current field without
//               actually unpacking any data or performing any range
//               validation.  Returns true on success, false on
//               failure (e.g. we don't know how to skip this field).
////////////////////////////////////////////////////////////////////
bool DCSimpleParameter::
unpack_skip(const char *data, size_t length, size_t &p, 
            bool &pack_error) const {
  size_t string_length;

  switch (_type) {
  case ST_char:
  case ST_int8:
  case ST_uint8:
    p++;
    break;

  case ST_int16:
  case ST_uint16:
    p += 2;
    break;

  case ST_int32:
  case ST_uint32:
    p += 4;
    break;

  case ST_int64:
  case ST_uint64:
  case ST_float64:
    p += 8;
    break;

  case ST_string:
  case ST_blob:
    if (_num_length_bytes == 0) {
      p += _fixed_byte_size;

    } else {
      if (p + 2 > length) {
        return false;
      }
      string_length = do_unpack_uint16(data + p);
      p += 2 + string_length;
    }
    break;

  case ST_blob32:
    if (_num_length_bytes == 0) {
      p += _fixed_byte_size;

    } else {
      if (p + 4 > length) {
        return false;
      }
      string_length = do_unpack_uint32(data + p);
      p += 4 + string_length;
    }
    break;

  default:
    return false;
  }

  if (p > length) {
    pack_error = true;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::output_instance
//       Access: Public, Virtual
//  Description: Formats the parameter in the C++-like dc syntax as a
//               typename and identifier.
////////////////////////////////////////////////////////////////////
void DCSimpleParameter::
output_instance(ostream &out, bool brief, const string &prename,
                const string &name, const string &postname) const {
  if (get_typedef() != (DCTypedef *)NULL) {
    output_typedef_name(out, brief, prename, name, postname);

  } else {
    out << _type;
    if (_has_modulus) {
      out << "%" << _orig_modulus;
    }
    if (_divisor != 1) {
      out << "/" << _divisor;
    }

    switch (_type) {
    case ST_int8:
    case ST_int16:
    case ST_int32:
      if (!_int_range.is_empty()) {
        out << "(";
        _int_range.output(out, _divisor);
        out << ")";
      }
      break;
    
    case ST_int64:
      if (!_int64_range.is_empty()) {
        out << "(";
        _int64_range.output(out, _divisor);
        out << ")";
      }
      break;
    
    case ST_uint8:
    case ST_uint16:
    case ST_uint32:
      if (!_uint_range.is_empty()) {
        out << "(";
        _uint_range.output(out, _divisor);
        out << ")";
      }
      break;

    case ST_char:
      if (!_uint_range.is_empty()) {
        out << "(";
        _uint_range.output_char(out, _divisor);
        out << ")";
      }
      break;
    
    case ST_uint64:
      if (!_uint64_range.is_empty()) {
        out << "(";
        _uint64_range.output(out, _divisor);
        out << ")";
      }
      break;

    case ST_float64:
      if (!_double_range.is_empty()) {
        out << "(";
        _double_range.output(out, _divisor);
        out << ")";
      }
      break;

    case ST_string:
      if (!_uint_range.is_empty()) {
        out << "(";
        _uint_range.output(out, _divisor);
        out << ")";
      }
      break;
    default:
      break;
    }

    if (!prename.empty() || !name.empty() || !postname.empty()) {
      out << " " << prename << name << postname;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::generate_hash
//       Access: Public, Virtual
//  Description: Accumulates the properties of this type into the
//               hash.
////////////////////////////////////////////////////////////////////
void DCSimpleParameter::
generate_hash(HashGenerator &hashgen) const {
  DCParameter::generate_hash(hashgen);

  hashgen.add_int(_type);
  hashgen.add_int(_divisor);
  if (_has_modulus) {
    hashgen.add_int((int)_double_modulus);
  }

  _int_range.generate_hash(hashgen);
  _int64_range.generate_hash(hashgen);
  _uint_range.generate_hash(hashgen);
  _uint64_range.generate_hash(hashgen);
  _double_range.generate_hash(hashgen);
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::do_check_match
//       Access: Protected, Virtual
//  Description: Returns true if the other interface is bitwise the
//               same as this one--that is, a uint32 only matches a
//               uint32, etc. Names of components, and range limits,
//               are not compared.
////////////////////////////////////////////////////////////////////
bool DCSimpleParameter::
do_check_match(const DCPackerInterface *other) const {
  return other->do_check_match_simple_parameter(this);
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::do_check_match_simple_parameter
//       Access: Protected, Virtual
//  Description: Returns true if this field matches the indicated
//               simple parameter, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCSimpleParameter::
do_check_match_simple_parameter(const DCSimpleParameter *other) const {
  if (_divisor != other->_divisor) {
    return false;
  }

  if (_type == other->_type) {
    return true;
  }

  // Check for certain types that are considered equivalent to each
  // other.
  switch (_type) {
  case ST_uint8:
  case ST_char:
    switch (other->_type) {
    case ST_uint8:
    case ST_char:
      return true;

    default:
      return false;
    }

  case ST_string:
  case ST_blob:
  case ST_uint8array:
    switch (other->_type) {
    case ST_string:
    case ST_blob:
    case ST_uint8array:
      return true;

    default:
      return false;
    }

  default:
    return false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::do_check_match_array_parameter
//       Access: Protected, Virtual
//  Description: Returns true if this field matches the indicated
//               array parameter, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCSimpleParameter::
do_check_match_array_parameter(const DCArrayParameter *other) const {
  if (other->get_array_size() != -1) {
    // We cannot match a fixed-size array.
    return false;
  }
  if (_nested_field == NULL) {
    // Only an array-style simple parameter can match a DCArrayParameter.
    return false;
  }

  return _nested_field->check_match(other->get_element_type());
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::create_nested_field
//       Access: Private, Static
//  Description: Creates the one instance of the DCSimpleParameter
//               corresponding to this combination of type and divisor
//               if it is not already created.
////////////////////////////////////////////////////////////////////
DCSimpleParameter *DCSimpleParameter::
create_nested_field(DCSubatomicType type, unsigned int divisor) {
  DivisorMap &divisor_map = _nested_field_map[type];
  DivisorMap::iterator di;
  di = divisor_map.find(divisor);
  if (di != divisor_map.end()) {
    return (*di).second;
  }

  DCSimpleParameter *nested_field = new DCSimpleParameter(type, divisor);
  divisor_map[divisor] = nested_field;
  return nested_field;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::create_uint32uint8_type
//       Access: Private, Static
//  Description: Creates the one instance of the Uint32Uint8Type
//               object if it is not already created.
////////////////////////////////////////////////////////////////////
DCPackerInterface *DCSimpleParameter::
create_uint32uint8_type() {
  if (_uint32uint8_type == NULL) {
    DCClass *dclass = new DCClass(NULL, "", true, false);
    dclass->add_field(new DCSimpleParameter(ST_uint32));
    dclass->add_field(new DCSimpleParameter(ST_uint8));
    _uint32uint8_type = new DCClassParameter(dclass);
  }
  return _uint32uint8_type;
}
