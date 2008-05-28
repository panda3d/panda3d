// Filename: dcPackerInterface.I
// Created by:  drose (18Jun04)
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


////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::get_name
//       Access: Published
//  Description: Returns the name of this field, or empty string
//               if the field is unnamed.
////////////////////////////////////////////////////////////////////
INLINE const string &DCPackerInterface::
get_name() const {
  return _name;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::check_match
//       Access: Published
//  Description: Returns true if the other interface is bitwise the
//               same as this one--that is, a uint32 only matches a
//               uint32, etc. Names of components, and range limits,
//               are not compared.
////////////////////////////////////////////////////////////////////
INLINE bool DCPackerInterface::
check_match(const DCPackerInterface *other) const {
  return do_check_match(other);
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::has_fixed_byte_size
//       Access: Public
//  Description: Returns true if this field type always packs to the
//               same number of bytes, false if it is variable.
////////////////////////////////////////////////////////////////////
INLINE bool DCPackerInterface::
has_fixed_byte_size() const {
  return _has_fixed_byte_size;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::get_fixed_byte_size
//       Access: Public
//  Description: If has_fixed_byte_size() returns true, this returns
//               the number of bytes this field type will use.
////////////////////////////////////////////////////////////////////
INLINE size_t DCPackerInterface::
get_fixed_byte_size() const {
  return _fixed_byte_size;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::has_fixed_structure
//       Access: Public
//  Description: Returns true if this field type always has the same
//               structure regardless of the data in the stream, or
//               false if its structure may vary.  This is almost, but
//               not quite, the same thing as has_fixed_byte_size.
//               The difference is that a DCSwitch may have multiple
//               cases all with the same byte size, but they will
//               still (presumably) have different structures, in the
//               sense that the actual list of fields varies according
//               to the live data.
////////////////////////////////////////////////////////////////////
INLINE bool DCPackerInterface::
has_fixed_structure() const {
  return _has_fixed_structure;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::has_range_limits
//       Access: Public
//  Description: Returns true if this field, or any sub-field of this
//               field, has a limit imposed in the DC file on its
//               legal values.  If this is false, then
//               unpack_validate() is trivial.
////////////////////////////////////////////////////////////////////
INLINE bool DCPackerInterface::
has_range_limits() const {
  return _has_range_limits;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::get_num_length_bytes
//       Access: Public
//  Description: Returns the number of bytes that should be written
//               into the stream on a push() to record the number of
//               bytes in the record up until the next pop().  This is
//               only meaningful if _has_nested_fields is true.
////////////////////////////////////////////////////////////////////
INLINE size_t DCPackerInterface::
get_num_length_bytes() const {
  return _num_length_bytes;
}


////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::has_nested_fields
//       Access: Public
//  Description: Returns true if this field type has any nested fields
//               (and thus expects a push() .. pop() interface to the
//               DCPacker), or false otherwise.  If this returns true,
//               get_num_nested_fields() may be called to determine
//               how many nested fields are expected.
////////////////////////////////////////////////////////////////////
INLINE bool DCPackerInterface::
has_nested_fields() const {
  return _has_nested_fields;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::get_num_nested_fields
//       Access: Public
//  Description: Returns the number of nested fields required by this
//               field type.  These may be array elements or structure
//               elements.  The return value may be -1 to indicate the
//               number of nested fields is variable.
////////////////////////////////////////////////////////////////////
INLINE int DCPackerInterface::
get_num_nested_fields() const {
  return _num_nested_fields;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::get_pack_type
//       Access: Public
//  Description: Returns the type of value expected by this field.
////////////////////////////////////////////////////////////////////
INLINE DCPackType DCPackerInterface::
get_pack_type() const {
  return _pack_type;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::do_pack_int8
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void DCPackerInterface::
do_pack_int8(char *buffer, int value) {
  buffer[0] = (char)(value & 0xff);
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::do_pack_int16
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void DCPackerInterface::
do_pack_int16(char *buffer, int value) {
  buffer[0] = (char)(value & 0xff);
  buffer[1] = (char)((value >> 8) & 0xff);
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::do_pack_int32
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void DCPackerInterface::
do_pack_int32(char *buffer, int value) {
  buffer[0] = (char)(value & 0xff);
  buffer[1] = (char)((value >> 8) & 0xff);
  buffer[2] = (char)((value >> 16) & 0xff);
  buffer[3] = (char)((value >> 24) & 0xff);
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::do_pack_int64
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void DCPackerInterface::
do_pack_int64(char *buffer, PN_int64 value) {
  buffer[0] = (char)(value & 0xff);
  buffer[1] = (char)((value >> 8) & 0xff);
  buffer[2] = (char)((value >> 16) & 0xff);
  buffer[3] = (char)((value >> 24) & 0xff);
  buffer[4] = (char)((value >> 32) & 0xff);
  buffer[5] = (char)((value >> 40) & 0xff);
  buffer[6] = (char)((value >> 48) & 0xff);
  buffer[7] = (char)((value >> 56) & 0xff);
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::do_pack_uint8
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void DCPackerInterface::
do_pack_uint8(char *buffer, unsigned int value) {
  buffer[0] = (char)(value & 0xff);
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::do_pack_uint16
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void DCPackerInterface::
do_pack_uint16(char *buffer, unsigned int value) {
  buffer[0] = (char)(value & 0xff);
  buffer[1] = (char)((value >> 8) & 0xff);
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::do_pack_uint32
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void DCPackerInterface::
do_pack_uint32(char *buffer, unsigned int value) {
  buffer[0] = (char)(value & 0xff);
  buffer[1] = (char)((value >> 8) & 0xff);
  buffer[2] = (char)((value >> 16) & 0xff);
  buffer[3] = (char)((value >> 24) & 0xff);
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::do_pack_uint64
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void DCPackerInterface::
do_pack_uint64(char *buffer, PN_uint64 value) {
  buffer[0] = (char)(value & 0xff);
  buffer[1] = (char)((value >> 8) & 0xff);
  buffer[2] = (char)((value >> 16) & 0xff);
  buffer[3] = (char)((value >> 24) & 0xff);
  buffer[4] = (char)((value >> 32) & 0xff);
  buffer[5] = (char)((value >> 40) & 0xff);
  buffer[6] = (char)((value >> 48) & 0xff);
  buffer[7] = (char)((value >> 56) & 0xff);
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::do_pack_float64
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void DCPackerInterface::
do_pack_float64(char *buffer, double value) {
#ifdef WORDS_BIGENDIAN
  // Reverse the byte ordering for big-endian machines.
  char *p = (char *)&value;
  for (size_t i = 0; i < 8; i++) {
    buffer[i] = p[7 - i];
  }
#else
  memcpy(buffer, &value, 8);
#endif
}


////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::do_unpack_int8
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int DCPackerInterface::
do_unpack_int8(const char *buffer) {
  return (int)(signed char)buffer[0];
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::do_unpack_int16
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int DCPackerInterface::
do_unpack_int16(const char *buffer) {
  return (int)((unsigned int)(unsigned char)buffer[0] |
               ((int)(signed char)buffer[1] << 8));
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::do_unpack_int32
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int DCPackerInterface::
do_unpack_int32(const char *buffer) {
  return (int)((unsigned int)(unsigned char)buffer[0] |
               ((unsigned int)(unsigned char)buffer[1] << 8) |
               ((unsigned int)(unsigned char)buffer[2] << 16) |
               ((int)(signed char)buffer[3] << 24));
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::do_unpack_int64
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PN_int64 DCPackerInterface::
do_unpack_int64(const char *buffer) {
  return (PN_int64)((PN_uint64)(unsigned char)buffer[0] |
                    ((PN_uint64)(unsigned char)buffer[1] << 8) |
                    ((PN_uint64)(unsigned char)buffer[2] << 16) |
                    ((PN_uint64)(unsigned char)buffer[3] << 24) |
                    ((PN_uint64)(unsigned char)buffer[4] << 32) |
                    ((PN_uint64)(unsigned char)buffer[5] << 40) |
                    ((PN_uint64)(unsigned char)buffer[6] << 48) |
                    ((PN_int64)(signed char)buffer[7] << 54));
}
////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::do_unpack_uint8
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE unsigned int DCPackerInterface::
do_unpack_uint8(const char *buffer) {
  return (unsigned int)(unsigned char)buffer[0];
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::do_unpack_uint16
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE unsigned int DCPackerInterface::
do_unpack_uint16(const char *buffer) {
  return ((unsigned int)(unsigned char)buffer[0] |
          ((unsigned int)(unsigned char)buffer[1] << 8));
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::do_unpack_uint32
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE unsigned int DCPackerInterface::
do_unpack_uint32(const char *buffer) {
  return ((unsigned int)(unsigned char)buffer[0] |
          ((unsigned int)(unsigned char)buffer[1] << 8) |
          ((unsigned int)(unsigned char)buffer[2] << 16) |
          ((unsigned int)(unsigned char)buffer[3] << 24));
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::do_unpack_uint64
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PN_uint64 DCPackerInterface::
do_unpack_uint64(const char *buffer) {
  return ((PN_uint64)(unsigned char)buffer[0] |
          ((PN_uint64)(unsigned char)buffer[1] << 8) |
          ((PN_uint64)(unsigned char)buffer[2] << 16) |
          ((PN_uint64)(unsigned char)buffer[3] << 24) |
          ((PN_uint64)(unsigned char)buffer[4] << 32) |
          ((PN_uint64)(unsigned char)buffer[5] << 40) |
          ((PN_uint64)(unsigned char)buffer[6] << 48) |
          ((PN_int64)(signed char)buffer[7] << 54));
}


////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::do_unpack_float64
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE double DCPackerInterface::
do_unpack_float64(const char *buffer) {
#ifdef WORDS_BIGENDIAN
  char reverse[8];
  
  // Reverse the byte ordering for big-endian machines.
  for (size_t i = 0; i < 8; i++) {
    reverse[i] = buffer[7 - i];
  }
  return *(double *)reverse;
#else
  return *(double *)buffer;
#endif  // WORDS_BIGENDIAN 
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::validate_int_limits
//       Access: Public, Static
//  Description: Confirms that the signed value fits within num_bits
//               bits.  Sets range_error true if it does not.
////////////////////////////////////////////////////////////////////
INLINE void DCPackerInterface::
validate_int_limits(int value, int num_bits, bool &range_error) {
  // What we're really checking is that all of the bits above the
  // lower (num_bits - 1) bits are the same--either all 1 or all 0.

  // First, turn on the lower (num_bits - 1).
  int mask = ((int)1 << (num_bits - 1)) - 1;
  value |= mask;

  // The result should be either mask (all high bits are 0) or -1 (all
  // high bits are 1).  If it is anything else we have a range error.
  if (value != mask && value != -1) {
    range_error = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::validate_int64_limits
//       Access: Public, Static
//  Description: Confirms that the signed value fits within num_bits
//               bits.  Sets range_error true if it does not.
////////////////////////////////////////////////////////////////////
INLINE void DCPackerInterface::
validate_int64_limits(PN_int64 value, int num_bits, bool &range_error) {
  PN_int64 mask = ((PN_int64)1 << (num_bits - 1)) - 1;
  value |= mask;

  if (value != mask && value != -1) {
    range_error = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::validate_uint_limits
//       Access: Public, Static
//  Description: Confirms that the unsigned value fits within num_bits
//               bits.  Sets range_error true if it does not.
////////////////////////////////////////////////////////////////////
INLINE void DCPackerInterface::
validate_uint_limits(unsigned int value, int num_bits, bool &range_error) {
  // Here we're really checking that all of the bits above the lower
  // num_bits bits are all 0.

  unsigned int mask = ((unsigned int)1 << num_bits) - 1;
  value &= ~mask;

  if (value != 0) {
    range_error = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::validate_uint64_limits
//       Access: Public, Static
//  Description: Confirms that the unsigned value fits within num_bits
//               bits.  Sets range_error true if it does not.
////////////////////////////////////////////////////////////////////
INLINE void DCPackerInterface::
validate_uint64_limits(PN_uint64 value, int num_bits, bool &range_error) {
  PN_uint64 mask = ((PN_uint64)1 << num_bits) - 1;
  value &= ~mask;

  if (value != 0) {
    range_error = true;
  }
}
