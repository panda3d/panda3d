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
//     Function: DCPackerInterface::has_nested_fields
//       Access: Public, Virtual
//  Description: Returns true if this field type has any nested fields
//               (and thus expects a push() .. pop() interface to the
//               DCPacker), or false otherwise.  If this returns true,
//               get_num_nested_fields() may be called to determine
//               how many nested fields are expected.
////////////////////////////////////////////////////////////////////
bool DCPackerInterface::
has_nested_fields() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::get_num_nested_fields
//       Access: Public, Virtual
//  Description: Returns the number of nested fields required by this
//               field type.  These may be array elements or structure
//               elements.  The return value may be -1 to indicate the
//               number of nested fields is variable.
////////////////////////////////////////////////////////////////////
int DCPackerInterface::
get_num_nested_fields() const {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::get_num_nested_fields
//       Access: Public, Virtual
//  Description: This flavor of get_num_nested_fields is used during
//               unpacking.  It returns the number of nested fields to
//               expect, given a certain length in bytes (as read from
//               the get_length_bytes() stored in the stream on the
//               pack).  This will only be called if
//               get_length_bytes() returns nonzero.
////////////////////////////////////////////////////////////////////
int DCPackerInterface::
get_num_nested_fields(size_t length_bytes) const {
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
//     Function: DCPackerInterface::get_length_bytes
//       Access: Public, Virtual
//  Description: If has_nested_fields() returns true, this should
//               return either 0, 2, or 4, indicating the number of
//               bytes this field's data should be prefixed with to
//               record its length.  This is respected by push() and
//               pop().
////////////////////////////////////////////////////////////////////
size_t DCPackerInterface::
get_length_bytes() const {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::get_pack_type
//       Access: Public, Virtual
//  Description: Returns the type of value expected by this field.
////////////////////////////////////////////////////////////////////
DCPackType DCPackerInterface::
get_pack_type() const {
  return PT_invalid;
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
//     Function: DCPackerInterface::unpack_string
//       Access: Public, Virtual
//  Description: Unpacks the current numeric or string value from the
//               stream.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCPackerInterface::
unpack_string(const char *, size_t, size_t &, string &) const {
  return false;
}
