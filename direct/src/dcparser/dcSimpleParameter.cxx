// Filename: dcSimpleParameter.cxx
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

#include "dcSimpleParameter.h"
#include "dcPackData.h"
#include "hashGenerator.h"

DCSimpleParameter::NestedFieldMap DCSimpleParameter::_nested_field_map;
DCSimpleParameter::Uint32Uint8Type *DCSimpleParameter::_uint32uint8_type = NULL;

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
DCSimpleParameter::
DCSimpleParameter(DCSubatomicType type, int divisor) :
  _type(type),
  _divisor(divisor)
{
  // Check for one of the built-in array types.  For these types, we
  // must present a packing interface that has a variable number of
  // nested fields of the appropriate type.
  switch (_type) {
  case ST_int8array:
    _pack_type = PT_array;
    _nested_type = ST_int8;
    _is_array = true;
    _bytes_per_element = 1;
    break;

  case ST_int16array:
    _pack_type = PT_array;
    _nested_type = ST_int16;
    _is_array = true;
    _bytes_per_element = 2;
    break;

  case ST_int32array:
    _pack_type = PT_array;
    _nested_type = ST_int32;
    _is_array = true;
    _bytes_per_element = 4;
    break;

  case ST_uint8array:
    _pack_type = PT_array;
    _nested_type = ST_uint8;
    _is_array = true;
    _bytes_per_element = 1;
    break;

  case ST_uint16array:
    _pack_type = PT_array;
    _nested_type = ST_uint16;
    _is_array = true;
    _bytes_per_element = 2;
    break;

  case ST_uint32array:
    _pack_type = PT_array;
    _nested_type = ST_uint32;
    _is_array = true;
    _bytes_per_element = 4;
    break;

  case ST_uint32uint8array:
    _pack_type = PT_array;
    _nested_type = ST_invalid;
    _is_array = true;
    _bytes_per_element = 5;
    break;

  case ST_blob:
  case ST_blob32:
  case ST_string: 
    // For these types, we will present an array interface as an array
    // of uint8, but we will also accept a set_value() with a string
    // parameter.
    _pack_type = PT_string;
    _nested_type = ST_uint8;
    _is_array = true;
    _bytes_per_element = 1;
    break;

    // The simple types can be packed directly.
  case ST_int8:
    _pack_type = PT_int;
    _nested_type = ST_invalid;
    _is_array = false;
    _bytes_per_element = 0;
    break;

  case ST_int16:
    _pack_type = PT_int;
    _nested_type = ST_invalid;
    _is_array = false;
    _bytes_per_element = 0;
    break;

  case ST_int32:
    _pack_type = PT_int;
    _nested_type = ST_invalid;
    _is_array = false;
    _bytes_per_element = 0;
    break;

  case ST_int64:
    _pack_type = PT_int64;
    _nested_type = ST_invalid;
    _is_array = false;
    _bytes_per_element = 0;
    break;

  case ST_uint8:
    _pack_type = PT_int;
    _nested_type = ST_invalid;
    _is_array = false;
    _bytes_per_element = 0;
    break;

  case ST_uint16:
    _pack_type = PT_int;
    _nested_type = ST_invalid;
    _is_array = false;
    _bytes_per_element = 0;
    break;

  case ST_uint32:
    _pack_type = PT_int;
    _nested_type = ST_invalid;
    _is_array = false;
    _bytes_per_element = 0;
    break;

  case ST_uint64:
    _pack_type = PT_int64;
    _nested_type = ST_invalid;
    _is_array = false;
    _bytes_per_element = 0;
    break;

  case ST_float64:
    _pack_type = PT_double;
    _nested_type = ST_invalid;
    _is_array = false;
    _bytes_per_element = 0;
    break;

  case ST_invalid:
    _pack_type = PT_invalid;
    _nested_type = ST_invalid;
    _is_array = false;
    _bytes_per_element = 0;
  }

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
//     Function: DCSimpleParameter::as_simple_parameter
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
DCSimpleParameter *DCSimpleParameter::
as_simple_parameter() {
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
//     Function: DCSimpleParameter::set_divisor
//       Access: Published
//  Description: See get_divisor().
////////////////////////////////////////////////////////////////////
void DCSimpleParameter::
set_divisor(int divisor) {
  _divisor = divisor;
  if (_pack_type == PT_int || _pack_type == PT_int64) {
    _pack_type = PT_double;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::has_nested_fields
//       Access: Public, Virtual
//  Description: Returns true if this field type has any nested fields
//               (and thus expects a push() .. pop() interface to the
//               DCPacker), or false otherwise.  If this returns true,
//               get_num_nested_fields() may be called to determine
//               how many nested fields are expected.
////////////////////////////////////////////////////////////////////
bool DCSimpleParameter::
has_nested_fields() const {
  return _is_array;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::get_num_nested_fields
//       Access: Public, Virtual
//  Description: Returns the number of nested fields required by this
//               field type.  These may be array elements or structure
//               elements.  The return value may be -1 to indicate the
//               number of nested fields is variable.
////////////////////////////////////////////////////////////////////
int DCSimpleParameter::
get_num_nested_fields() const {
  return _is_array ? -1 : 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::get_num_nested_fields
//       Access: Public, Virtual
//  Description: This flavor of get_num_nested_fields is used during
//               unpacking.  It returns the number of nested fields to
//               expect, given a certain length in bytes (as read from
//               the get_length_bytes() stored in the stream on the
//               pack).  This will only be called if
//               get_length_bytes() returns nonzero.
////////////////////////////////////////////////////////////////////
int DCSimpleParameter::
get_num_nested_fields(size_t length_bytes) const {
  if (_is_array) {
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
get_nested_field(int n) const {
  return _nested_field;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::get_length_bytes
//       Access: Public, Virtual
//  Description: If has_nested_fields() returns true, this should
//               return either 0, 2, or 4, indicating the number of
//               bytes this field's data should be prefixed with to
//               record its length.  This is respected by push() and
//               pop().
////////////////////////////////////////////////////////////////////
size_t DCSimpleParameter::
get_length_bytes() const {
  return _type == ST_blob32 ? 4 : 2;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::get_pack_type
//       Access: Public, Virtual
//  Description: Returns the type of value expected by this field.
////////////////////////////////////////////////////////////////////
DCPackType DCSimpleParameter::
get_pack_type() const {
  return _pack_type;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::pack_double
//       Access: Published, Virtual
//  Description: Packs the indicated numeric or string value into the
//               stream.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCSimpleParameter::
pack_double(DCPackData &pack_data, double value) const {
  double real_value = value * _divisor;
  int int_value = (int)floor(real_value + 0.5);

  char buffer[8];

  switch (_type) {
  case ST_int8:
  case ST_uint8:
    buffer[0] = (char)(int_value & 0xff);
    pack_data.append_data(buffer, 1);
    break;

  case ST_int16:
  case ST_uint16:
    buffer[0] = (char)(int_value & 0xff);
    buffer[1] = (char)((int_value >> 8) & 0xff);
    pack_data.append_data(buffer, 2);
    break;

  case ST_int32:
  case ST_uint32:
    buffer[0] = (char)(int_value & 0xff);
    buffer[1] = (char)((int_value >> 8) & 0xff);
    buffer[2] = (char)((int_value >> 16) & 0xff);
    buffer[3] = (char)((int_value >> 24) & 0xff);
    pack_data.append_data(buffer, 4);
    break;

  case ST_int64:
  case ST_uint64:
    {
      PN_int64 int64_value = (PN_int64)floor(real_value + 0.5);
      buffer[0] = (char)(int64_value & 0xff);
      buffer[1] = (char)((int64_value >> 8) & 0xff);
      buffer[2] = (char)((int64_value >> 16) & 0xff);
      buffer[3] = (char)((int64_value >> 24) & 0xff);
      buffer[4] = (char)((int64_value >> 32) & 0xff);
      buffer[5] = (char)((int64_value >> 40) & 0xff);
      buffer[6] = (char)((int64_value >> 48) & 0xff);
      buffer[7] = (char)((int64_value >> 56) & 0xff);
      pack_data.append_data(buffer, 8);
    }
    break;

  case ST_float64:
#ifdef WORDS_BIGENDIAN
    {
      // Reverse the byte ordering for big-endian machines.
      char *p = (char *)real_value;
      for (size_t i = 0; i < 8; i++) {
        buffer[i] = p[7 - i];
      }
    }
#else
    memcpy(buffer, &real_value, 8);
#endif
    pack_data.append_data(buffer, 8);
    break;

  default:
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::pack_int
//       Access: Published, Virtual
//  Description: Packs the indicated numeric or string value into the
//               stream.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCSimpleParameter::
pack_int(DCPackData &pack_data, int value) const {
  int int_value = value * _divisor;

  char buffer[8];

  switch (_type) {
  case ST_int8:
  case ST_uint8:
    buffer[0] = (char)(int_value & 0xff);
    pack_data.append_data(buffer, 1);
    break;

  case ST_int16:
  case ST_uint16:
    buffer[0] = (char)(int_value & 0xff);
    buffer[1] = (char)((int_value >> 8) & 0xff);
    pack_data.append_data(buffer, 2);
    break;

  case ST_int32:
  case ST_uint32:
    buffer[0] = (char)(int_value & 0xff);
    buffer[1] = (char)((int_value >> 8) & 0xff);
    buffer[2] = (char)((int_value >> 16) & 0xff);
    buffer[3] = (char)((int_value >> 24) & 0xff);
    pack_data.append_data(buffer, 4);
    break;

  case ST_int64:
    buffer[0] = (char)(int_value & 0xff);
    buffer[1] = (char)((int_value >> 8) & 0xff);
    buffer[2] = (char)((int_value >> 16) & 0xff);
    buffer[3] = (char)((int_value >> 24) & 0xff);
    if ((int_value & 0x80000000) != 0) {
      buffer[4] = buffer[5] = buffer[6] = buffer[7] = (char)0xff;
    } else {
      buffer[4] = buffer[5] = buffer[6] = buffer[7] = (char)0;
    }
    pack_data.append_data(buffer, 8);
    break;

  case ST_uint64:
    buffer[0] = (char)(int_value & 0xff);
    buffer[1] = (char)((int_value >> 8) & 0xff);
    buffer[2] = (char)((int_value >> 16) & 0xff);
    buffer[3] = (char)((int_value >> 24) & 0xff);
    buffer[4] = buffer[5] = buffer[6] = buffer[7] = (char)0;
    pack_data.append_data(buffer, 8);
    break;

  case ST_float64:
#ifdef WORDS_BIGENDIAN
    {
      // Reverse the byte ordering for big-endian machines.
      double real_value = int_value;
      char *p = (char *)real_value;
      for (size_t i = 0; i < 8; i++) {
        buffer[i] = p[7 - i];
      }
    }
#else
    {
      double real_value = int_value;
      memcpy(buffer, &real_value, 8);
    }
#endif
    pack_data.append_data(buffer, 8);
    break;

  default:
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::pack_int64
//       Access: Published, Virtual
//  Description: Packs the indicated numeric or string value into the
//               stream.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCSimpleParameter::
pack_int64(DCPackData &pack_data, PN_int64 value) const {
  PN_int64 int_value = value * _divisor;

  char buffer[8];

  switch (_type) {
  case ST_int8:
  case ST_uint8:
    buffer[0] = (char)(int_value & 0xff);
    pack_data.append_data(buffer, 1);
    break;

  case ST_int16:
  case ST_uint16:
    buffer[0] = (char)(int_value & 0xff);
    buffer[1] = (char)((int_value >> 8) & 0xff);
    pack_data.append_data(buffer, 2);
    break;

  case ST_int32:
  case ST_uint32:
    buffer[0] = (char)(int_value & 0xff);
    buffer[1] = (char)((int_value >> 8) & 0xff);
    buffer[2] = (char)((int_value >> 16) & 0xff);
    buffer[3] = (char)((int_value >> 24) & 0xff);
    pack_data.append_data(buffer, 4);
    break;

  case ST_int64:
  case ST_uint64:
    {
      buffer[0] = (char)(int_value & 0xff);
      buffer[1] = (char)((int_value >> 8) & 0xff);
      buffer[2] = (char)((int_value >> 16) & 0xff);
      buffer[3] = (char)((int_value >> 24) & 0xff);
      buffer[4] = (char)((int_value >> 32) & 0xff);
      buffer[5] = (char)((int_value >> 40) & 0xff);
      buffer[6] = (char)((int_value >> 48) & 0xff);
      buffer[7] = (char)((int_value >> 56) & 0xff);
      pack_data.append_data(buffer, 8);
    }
    break;

  case ST_float64:
#ifdef WORDS_BIGENDIAN
    {
      // Reverse the byte ordering for big-endian machines.
      double real_value = int_value;
      char *p = (char *)real_value;
      for (size_t i = 0; i < 8; i++) {
        buffer[i] = p[7 - i];
      }
    }
#else
    {
      double real_value = int_value;
      memcpy(buffer, &real_value, 8);
    }
#endif
    pack_data.append_data(buffer, 8);
    break;

  default:
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::pack_string
//       Access: Published, Virtual
//  Description: Packs the indicated numeric or string value into the
//               stream.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCSimpleParameter::
pack_string(DCPackData &pack_data, const string &value) const {
  char buffer[4];

  switch (_type) {
  case ST_string:
  case ST_blob:
    {
      int length = value.length();
      buffer[0] = (char)(length & 0xff);
      buffer[1] = (char)((length >> 8) & 0xff);
      pack_data.append_data(buffer, 2);
      pack_data.append_data(value.data(), length);
    }
    break;

  case ST_blob32:
    {
      int length = value.length();
      buffer[0] = (char)(length & 0xff);
      buffer[1] = (char)((length >> 8) & 0xff);
      buffer[2] = (char)((length >> 16) & 0xff);
      buffer[3] = (char)((length >> 24) & 0xff);
      pack_data.append_data(buffer, 4);
      pack_data.append_data(value.data(), length);
    }
    break;

  default:
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::unpack_double
//       Access: Public, Virtual
//  Description: Unpacks the current numeric or string value from the
//               stream.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCSimpleParameter::
unpack_double(const char *data, size_t length, size_t &p, double &value) const {
  switch (_type) {
  case ST_int8:
    if (p + 1 > length) {
      return false;
    }
    value = (double)(int)(signed char)data[p];
    p++;
    break;

  case ST_int16:
    if (p + 2 > length) {
      return false;
    }
    value = (double)(int)((unsigned int)(unsigned char)data[p] |
                          ((int)(signed char)data[p + 1] << 8));
    p += 2;
    break;

  case ST_int32:
    if (p + 4 > length) {
      return false;
    }
    value = (double)(int)((unsigned int)(unsigned char)data[p] |
                          ((unsigned int)(unsigned char)data[p + 1] << 8) |
                          ((unsigned int)(unsigned char)data[p + 2] << 16) |
                          ((int)(signed char)data[p + 3] << 24));
    p += 4;
    break;

  case ST_int64:
    if (p + 8 > length) {
      return false;
    }
    value = (double)(PN_int64)((PN_uint64)(unsigned char)data[p] |
                               ((PN_uint64)(unsigned char)data[p + 1] << 8) |
                               ((PN_uint64)(unsigned char)data[p + 2] << 16) |
                               ((PN_uint64)(unsigned char)data[p + 3] << 24) |
                               ((PN_uint64)(unsigned char)data[p + 4] << 32) |
                               ((PN_uint64)(unsigned char)data[p + 5] << 40) |
                               ((PN_uint64)(unsigned char)data[p + 6] << 48) |
                               ((PN_int64)(signed char)data[p + 7] << 54));
    p += 8;
    break;

  case ST_uint8:
    if (p + 1 > length) {
      return false;
    }
    value = (double)(unsigned int)(unsigned char)data[p];
    p++;
    break;

  case ST_uint16:
    if (p + 2 > length) {
      return false;
    }
    value = (double)(unsigned int)((unsigned int)(unsigned char)data[p] |
                                   ((unsigned int)(unsigned char)data[p + 1] << 8));
    p += 2;
    break;

  case ST_uint32:
    if (p + 4 > length) {
      return false;
    }
    value = (double)(unsigned int)((unsigned int)(unsigned char)data[p] |
                                   ((unsigned int)(unsigned char)data[p + 1] << 8) |
                                   ((unsigned int)(unsigned char)data[p + 2] << 16) |
                                   ((unsigned int)(unsigned char)data[p + 3] << 24));
    p += 4;
    break;

  case ST_uint64:
    if (p + 8 > length) {
      return false;
    }
    value = (double)(PN_uint64)((PN_uint64)(unsigned char)data[p] |
                                ((PN_uint64)(unsigned char)data[p + 1] << 8) |
                                ((PN_uint64)(unsigned char)data[p + 2] << 16) |
                                ((PN_uint64)(unsigned char)data[p + 3] << 24) |
                                ((PN_uint64)(unsigned char)data[p + 4] << 32) |
                                ((PN_uint64)(unsigned char)data[p + 5] << 40) |
                                ((PN_uint64)(unsigned char)data[p + 6] << 48) |
                                ((PN_uint64)(unsigned char)data[p + 7] << 54));
    p += 8;
    break;

  case ST_float64:
    if (p + 8 > length) {
      return false;
    }
    {
      double *real_value;
#ifdef WORDS_BIGENDIAN
      char buffer[8];

      // Reverse the byte ordering for big-endian machines.
      for (size_t i = 0; i < 8; i++) {
        buffer[i] = data[p + 7 - i];
      }
      real_value = (double *)buffer;
#else
      real_value = (double *)(data + p);
#endif  // WORDS_BIGENDIAN 
      value = (*real_value);
    }
    p += 8;
    break;

  default:
    return false;
  }

  if (_divisor != 1) {
    value = value / _divisor;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::unpack_int
//       Access: Public, Virtual
//  Description: Unpacks the current numeric or string value from the
//               stream.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCSimpleParameter::
unpack_int(const char *data, size_t length, size_t &p, int &value) const {
  switch (_type) {
  case ST_int8:
    if (p + 1 > length) {
      return false;
    }
    value = (int)(signed char)data[p];
    p++;
    break;

  case ST_int16:
    if (p + 2 > length) {
      return false;
    }
    value = (int)((unsigned int)(unsigned char)data[p] |
                  ((int)(signed char)data[p + 1] << 8));
    p += 2;
    break;

  case ST_int32:
    if (p + 4 > length) {
      return false;
    }
    value = (int)((unsigned int)(unsigned char)data[p] |
                  ((unsigned int)(unsigned char)data[p + 1] << 8) |
                  ((unsigned int)(unsigned char)data[p + 2] << 16) |
                  ((int)(signed char)data[p + 3] << 24));
    p += 4;
    break;

  case ST_int64:
    if (p + 8 > length) {
      return false;
    }
    value = (int)((unsigned int)(unsigned char)data[p] |
                  ((unsigned int)(unsigned char)data[p + 1] << 8) |
                  ((unsigned int)(unsigned char)data[p + 2] << 16) |
                  ((unsigned int)(unsigned char)data[p + 3] << 24));
    p += 8;
    break;

  case ST_uint8:
    if (p + 1 > length) {
      return false;
    }
    value = (unsigned int)(unsigned char)data[p];
    p++;
    break;

  case ST_uint16:
    if (p + 2 > length) {
      return false;
    }
    value = (unsigned int)((unsigned int)(unsigned char)data[p] |
                           ((unsigned int)(unsigned char)data[p + 1] << 8));
    p += 2;
    break;

  case ST_uint32:
    if (p + 4 > length) {
      return false;
    }
    value = (unsigned int)((unsigned int)(unsigned char)data[p] |
                           ((unsigned int)(unsigned char)data[p + 1] << 8) |
                           ((unsigned int)(unsigned char)data[p + 2] << 16) |
                           ((unsigned int)(unsigned char)data[p + 3] << 24));
    p += 4;
    break;

  case ST_uint64:
    if (p + 8 > length) {
      return false;
    }
    value = (unsigned int)((unsigned int)(unsigned char)data[p] |
                           ((unsigned int)(unsigned char)data[p + 1] << 8) |
                           ((unsigned int)(unsigned char)data[p + 2] << 16) |
                           ((unsigned int)(unsigned char)data[p + 3] << 24));
    p += 8;
    break;

  case ST_float64:
    if (p + 8 > length) {
      return false;
    }
    {
      double *real_value;
#ifdef WORDS_BIGENDIAN
      char buffer[8];

      // Reverse the byte ordering for big-endian machines.
      for (size_t i = 0; i < 8; i++) {
        buffer[i] = data[p + 7 - i];
      }
      real_value = (double *)buffer;
#else
      real_value = (double *)(data + p);
#endif  // WORDS_BIGENDIAN 
      value = (int)(*real_value);
    }
    p += 8;
    break;

  default:
    return false;
  }

  if (_divisor != 1) {
    value = value / _divisor;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::unpack_int64
//       Access: Public, Virtual
//  Description: Unpacks the current numeric or string value from the
//               stream.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCSimpleParameter::
unpack_int64(const char *data, size_t length, size_t &p, PN_int64 &value) const {
  switch (_type) {
  case ST_int8:
    if (p + 1 > length) {
      return false;
    }
    value = (int)(signed char)data[p];
    p++;
    break;

  case ST_int16:
    if (p + 2 > length) {
      return false;
    }
    value = (int)((unsigned int)(unsigned char)data[p] |
                  ((int)(signed char)data[p + 1] << 8));
    p += 2;
    break;

  case ST_int32:
    if (p + 4 > length) {
      return false;
    }
    value = (int)((unsigned int)(unsigned char)data[p] |
                  ((unsigned int)(unsigned char)data[p + 1] << 8) |
                  ((unsigned int)(unsigned char)data[p + 2] << 16) |
                  ((int)(signed char)data[p + 3] << 24));
    p += 4;
    break;

  case ST_int64:
    if (p + 8 > length) {
      return false;
    }
    value = (PN_int64)((PN_uint64)(unsigned char)data[p] |
                       ((PN_uint64)(unsigned char)data[p + 1] << 8) |
                       ((PN_uint64)(unsigned char)data[p + 2] << 16) |
                       ((PN_uint64)(unsigned char)data[p + 3] << 24) |
                       ((PN_uint64)(unsigned char)data[p + 4] << 32) |
                       ((PN_uint64)(unsigned char)data[p + 5] << 40) |
                       ((PN_uint64)(unsigned char)data[p + 6] << 48) |
                       ((PN_int64)(signed char)data[p + 7] << 54));
    p += 8;
    break;

  case ST_uint8:
    if (p + 1 > length) {
      return false;
    }
    value = (unsigned int)(unsigned char)data[p];
    p++;
    break;

  case ST_uint16:
    if (p + 2 > length) {
      return false;
    }
    value = (unsigned int)((unsigned int)(unsigned char)data[p] |
                           ((unsigned int)(unsigned char)data[p + 1] << 8));
    p += 2;
    break;

  case ST_uint32:
    if (p + 4 > length) {
      return false;
    }
    value = (unsigned int)((unsigned int)(unsigned char)data[p] |
                           ((unsigned int)(unsigned char)data[p + 1] << 8) |
                           ((unsigned int)(unsigned char)data[p + 2] << 16) |
                           ((unsigned int)(unsigned char)data[p + 3] << 24));
    p += 4;
    break;

  case ST_uint64:
    if (p + 8 > length) {
      return false;
    }
    value = (PN_int64)((PN_uint64)(unsigned char)data[p] |
                       ((PN_uint64)(unsigned char)data[p + 1] << 8) |
                       ((PN_uint64)(unsigned char)data[p + 2] << 16) |
                       ((PN_uint64)(unsigned char)data[p + 3] << 24) |
                       ((PN_uint64)(unsigned char)data[p + 4] << 32) |
                       ((PN_uint64)(unsigned char)data[p + 5] << 40) |
                       ((PN_uint64)(unsigned char)data[p + 6] << 48) |
                       ((PN_uint64)(unsigned char)data[p + 7] << 54));
    p += 8;
    break;

  case ST_float64:
    if (p + 8 > length) {
      return false;
    }
    {
      double *real_value;
#ifdef WORDS_BIGENDIAN
      char buffer[8];

      // Reverse the byte ordering for big-endian machines.
      for (size_t i = 0; i < 8; i++) {
        buffer[i] = data[p + 7 - i];
      }
      real_value = (double *)buffer;
#else
      real_value = (double *)(data + p);
#endif  // WORDS_BIGENDIAN 
      value = (PN_int64)(*real_value);
    }
    p += 8;
    break;

  default:
    return false;
  }

  if (_divisor != 1) {
    value = value / _divisor;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::unpack_string
//       Access: Public, Virtual
//  Description: Unpacks the current numeric or string value from the
//               stream.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCSimpleParameter::
unpack_string(const char *data, size_t length, size_t &p, string &value) const {
  size_t string_length;

  switch (_type) {
  case ST_string:
  case ST_blob:
    if (p + 2 > length) {
      return false;
    }
    string_length = (size_t)((unsigned int)(unsigned char)data[p] |
                             ((unsigned int)(unsigned char)data[p + 1] << 8));
    p += 2;
    break;

  case ST_blob32:
    if (p + 4 > length) {
      return false;
    }
    string_length = (size_t)((unsigned int)(unsigned char)data[p] |
                             ((unsigned int)(unsigned char)data[p + 1] << 8) |
                             ((unsigned int)(unsigned char)data[p + 2] << 16) |
                             ((unsigned int)(unsigned char)data[p + 3] << 24));
    p += 4;
    break;

  default:
    return false;
  }

  if (p + string_length > length) {
    return false;
  }
  value = string(data + p, string_length);
  p += string_length;

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void DCSimpleParameter::
output(ostream &out, bool brief) const {
  out << _type;
  if (_divisor != 1) {
    out << " / " << _divisor;
  }
  if (!brief && !_name.empty()) {
    out << " " << _name;
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
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::create_nested_field
//       Access: Private, Static
//  Description: Creates the one instance of the DCSimpleParameter
//               corresponding to this combination of type and divisor
//               if it is not already created.
////////////////////////////////////////////////////////////////////
DCSimpleParameter *DCSimpleParameter::
create_nested_field(DCSubatomicType type, int divisor) {
  DivisorMap divisor_map = _nested_field_map[type];
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
    _uint32uint8_type = new Uint32Uint8Type;
  }
  return _uint32uint8_type;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::Uint32Uint8Type::Constructor
//       Access: Public
//  Description: This special packer interface is provided just to
//               implement uint32uint8array, which is a special kind
//               of array that consists of nested pairs of (uint32,
//               uint8) values.
////////////////////////////////////////////////////////////////////
DCSimpleParameter::Uint32Uint8Type::
Uint32Uint8Type() {
  _uint32_type = new DCSimpleParameter(ST_uint32);
  _uint8_type = new DCSimpleParameter(ST_uint8);
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::Uint32Uint8Type::has_nested_fields
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
bool DCSimpleParameter::Uint32Uint8Type::
has_nested_fields() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::Uint32Uint8Type::get_num_nested_fields
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
int DCSimpleParameter::Uint32Uint8Type::
get_num_nested_fields() const {
  return 2;
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::Uint32Uint8Type::get_nested_field
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
DCPackerInterface *DCSimpleParameter::Uint32Uint8Type::
get_nested_field(int n) const {
  switch (n) {
  case 0:
    return _uint32_type;

  case 1:
    return _uint8_type;

  default:
    return NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCSimpleParameter::Uint32Uint8Type::get_pack_type
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
DCPackType DCSimpleParameter::Uint32Uint8Type::
get_pack_type() const {
  return PT_struct;
}
