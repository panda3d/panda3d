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
//     Function: DCPackerInterface::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
DCPackerInterface::
~DCPackerInterface() {
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::get_num_nested_fields
//       Access: Public, Virtual
//  Description: Returns the number of nested fields required by this
//               field type.  These may be array elements or structure
//               elements.  The return value should be 0 if the field
//               type does not have any nested fields.  It may also be
//               -1 to indicate the number of nested fields is
//               variable.
////////////////////////////////////////////////////////////////////
int DCPackerInterface::
get_num_nested_fields() const {
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
//  Description: If get_num_nested_fields() returns non-zero, this
//               should return either 0, 2, or 4, indicating the
//               number of bytes this field's data should be prefixed
//               with to record its length.  This is respected by
//               push() and pop().
////////////////////////////////////////////////////////////////////
size_t DCPackerInterface::
get_length_bytes() const {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::get_pack_type
//       Access: Public, Virtual
//  Description: Returns the type of value expected by this field, or
//               ST_invalid if this field cannot accept simple value
//               types.
////////////////////////////////////////////////////////////////////
DCSubatomicType DCPackerInterface::
get_pack_type() const {
  return ST_invalid;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::pack_value
//       Access: Public, Virtual
//  Description: Packs the indicated numeric or string value into the
//               stream.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCPackerInterface::
pack_value(DCPackData &, double) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::pack_value
//       Access: Public, Virtual
//  Description: Packs the indicated numeric or string value into the
//               stream.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCPackerInterface::
pack_value(DCPackData &, int) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::pack_value
//       Access: Public, Virtual
//  Description: Packs the indicated numeric or string value into the
//               stream.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCPackerInterface::
pack_value(DCPackData &, PN_int64) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::pack_value
//       Access: Public, Virtual
//  Description: Packs the indicated numeric or string value into the
//               stream.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCPackerInterface::
pack_value(DCPackData &, const string &) const {
  return false;
}
