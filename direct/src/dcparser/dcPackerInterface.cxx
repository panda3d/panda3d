// Filename: dcPackerInterface.cxx
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

#include "dcPackerInterface.h"

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
DCPackerInterface::
DCPackerInterface(const string &name) :
  _name(name)
{
  _has_fixed_byte_size = false;
  _fixed_byte_size = 0;
  _num_length_bytes = 0;
  _has_nested_fields = false;
  _num_nested_fields = -1;
  _pack_type = PT_invalid;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
DCPackerInterface::
DCPackerInterface(const DCPackerInterface &copy) :
  _name(copy._name),
  _has_fixed_byte_size(copy._has_fixed_byte_size),
  _fixed_byte_size(copy._fixed_byte_size),
  _num_length_bytes(copy._num_length_bytes),
  _has_nested_fields(copy._has_nested_fields),
  _num_nested_fields(copy._num_nested_fields),
  _pack_type(copy._pack_type)
{
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
DCPackerInterface::
~DCPackerInterface() {
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::get_name
//       Access: Published
//  Description: Returns the name of this field, or empty string
//               if the field is unnamed.
////////////////////////////////////////////////////////////////////
const string &DCPackerInterface::
get_name() const {
  return _name;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::set_name
//       Access: Published
//  Description: Sets the name of this field.
////////////////////////////////////////////////////////////////////
void DCPackerInterface::
set_name(const string &name) {
  _name = name;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::has_fixed_byte_size
//       Access: Public
//  Description: Returns true if this field type always packs to the
//               same number of bytes, false if it is variable.
////////////////////////////////////////////////////////////////////
bool DCPackerInterface::
has_fixed_byte_size() const {
  return _has_fixed_byte_size;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::get_fixed_byte_size
//       Access: Public
//  Description: If has_fixed_byte_size() returns true, this returns
//               the number of bytes this field type will use.
////////////////////////////////////////////////////////////////////
size_t DCPackerInterface::
get_fixed_byte_size() const {
  return _fixed_byte_size;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::get_num_length_bytes
//       Access: Public
//  Description: Returns the number of bytes that should be written
//               into the stream on a push() to record the number of
//               bytes in the record up until the next pop().  This is
//               only meaningful if _has_nested_fields is true.
////////////////////////////////////////////////////////////////////
size_t DCPackerInterface::
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
bool DCPackerInterface::
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
int DCPackerInterface::
get_num_nested_fields() const {
  return _num_nested_fields;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::calc_num_nested_fields
//       Access: Public, Virtual
//  Description: This flavor of get_num_nested_fields is used during
//               unpacking.  It returns the number of nested fields to
//               expect, given a certain length in bytes (as read from
//               the _num_length_bytes stored in the stream on the
//               push).  This will only be called if _num_length_bytes
//               is nonzero.
////////////////////////////////////////////////////////////////////
int DCPackerInterface::
calc_num_nested_fields(size_t length_bytes) const {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::get_nested_field
//       Access: Public, Virtual
//  Description: Returns the DCPackerInterface object that represents
//               the nth nested field.  This may return NULL if there
//               is no such field (but it shouldn't do this if n is in
//               the range 0 <= n < get_num_nested_fields()).
////////////////////////////////////////////////////////////////////
DCPackerInterface *DCPackerInterface::
get_nested_field(int n) const {
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::get_pack_type
//       Access: Public
//  Description: Returns the type of value expected by this field.
////////////////////////////////////////////////////////////////////
DCPackType DCPackerInterface::
get_pack_type() const {
  return _pack_type;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::pack_double
//       Access: Public, Virtual
//  Description: Packs the indicated numeric or string value into the
//               stream.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCPackerInterface::
pack_double(DCPackData &, double) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::pack_int
//       Access: Public, Virtual
//  Description: Packs the indicated numeric or string value into the
//               stream.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCPackerInterface::
pack_int(DCPackData &, int) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::pack_uint
//       Access: Public, Virtual
//  Description: Packs the indicated numeric or string value into the
//               stream.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCPackerInterface::
pack_uint(DCPackData &, unsigned int) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::pack_int64
//       Access: Public, Virtual
//  Description: Packs the indicated numeric or string value into the
//               stream.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCPackerInterface::
pack_int64(DCPackData &, PN_int64) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::pack_uint64
//       Access: Public, Virtual
//  Description: Packs the indicated numeric or string value into the
//               stream.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCPackerInterface::
pack_uint64(DCPackData &, PN_uint64) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::pack_string
//       Access: Public, Virtual
//  Description: Packs the indicated numeric or string value into the
//               stream.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCPackerInterface::
pack_string(DCPackData &, const string &) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::unpack_double
//       Access: Public, Virtual
//  Description: Unpacks the current numeric or string value from the
//               stream.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCPackerInterface::
unpack_double(const char *, size_t, size_t &, double &) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::unpack_int
//       Access: Public, Virtual
//  Description: Unpacks the current numeric or string value from the
//               stream.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCPackerInterface::
unpack_int(const char *, size_t, size_t &, int &) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::unpack_uint
//       Access: Public, Virtual
//  Description: Unpacks the current numeric or string value from the
//               stream.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCPackerInterface::
unpack_uint(const char *, size_t, size_t &, unsigned int &) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::unpack_int64
//       Access: Public, Virtual
//  Description: Unpacks the current numeric or string value from the
//               stream.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCPackerInterface::
unpack_int64(const char *, size_t, size_t &, PN_int64 &) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::unpack_uint64
//       Access: Public, Virtual
//  Description: Unpacks the current numeric or string value from the
//               stream.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCPackerInterface::
unpack_uint64(const char *, size_t, size_t &, PN_uint64 &) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::unpack_string
//       Access: Public, Virtual
//  Description: Unpacks the current numeric or string value from the
//               stream.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCPackerInterface::
unpack_string(const char *, size_t, size_t &, string &) const {
  return false;
}
