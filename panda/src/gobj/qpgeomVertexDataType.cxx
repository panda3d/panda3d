// Filename: qpgeomVertexDataType.cxx
// Created by:  drose (06Mar05)
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

#include "qpgeomVertexDataType.h"

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexDataType::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomVertexDataType::
qpGeomVertexDataType(const InternalName *name, int num_components,
                     NumericType numeric_type, Contents contents,
                     int start) :
  _name(name),
  _num_components(num_components),
  _num_values(num_components),
  _numeric_type(numeric_type),
  _contents(contents),
  _start(start)
{
  nassertv(num_components > 0 && start >= 0);

  switch (numeric_type) {
  case NT_uint16:
    _component_bytes = 2;  // sizeof(PN_uint16)
    break;

  case NT_uint8:
    _component_bytes = 1;
    break;

  case NT_packed_8888:
    _component_bytes = 4;  // sizeof(PN_uint32)
    _num_values *= 4;
    break;

  case NT_float32:
    _component_bytes = 4;  // sizeof(PN_float32)
    break;
  }

  _total_bytes = _component_bytes * _num_components;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexDataType::error named constructor
//       Access: Published
//  Description: Returns a data type specifically to represent an
//               error condition.
////////////////////////////////////////////////////////////////////
const qpGeomVertexDataType &qpGeomVertexDataType::
error() {
  static qpGeomVertexDataType error_result
    (InternalName::get_error(), 1, NT_uint8, C_other, 0);
  return error_result;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexDataType::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexDataType::
output(ostream &out) const {
  out << *get_name() << "(" << get_num_components() << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexDataType::copy_records
//       Access: Published
//  Description: Copies and converts from one data type to another.
//               Copies the num_records records from the "from"
//               buffer, encoded with from_type, to the "to" buffer,
//               encoded with this current type, converting each one
//               as necessary.
////////////////////////////////////////////////////////////////////
void qpGeomVertexDataType::
copy_records(unsigned char *to, int to_stride,
             const unsigned char *from, int from_stride,
             const qpGeomVertexDataType *from_type,
             int num_records) const {
  // Temp for debugging.
  static ConfigVariableBool do_copy_generic("copy-generic", false);
  if (do_copy_generic) {
    copy_generic(to, to_stride, from, from_stride, from_type, num_records);
    return;
  }

  if (get_numeric_type() == from_type->get_numeric_type() &&
      get_num_values() == from_type->get_num_values()) {
      // An easy case.
    copy_no_convert(to, to_stride, from, from_stride, from_type, num_records);

  } else if (get_numeric_type() == NT_uint8 && from_type->get_numeric_type() == NT_packed_8888 &&
             get_num_values() == from_type->get_num_values()) {
    copy_argb_to_uint8(to, to_stride, from, from_stride, from_type, num_records);
  } else if (get_numeric_type() == NT_packed_8888 && from_type->get_numeric_type() == NT_uint8 &&
             get_num_values() == from_type->get_num_values()) {
    copy_uint8_to_argb(to, to_stride, from, from_stride, from_type, num_records);
  } else {
    copy_generic(to, to_stride, from, from_stride, from_type, num_records);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexDataType::copy_no_convert
//       Access: Private
//  Description: Quickly copies data without the need to convert it.
////////////////////////////////////////////////////////////////////
void qpGeomVertexDataType::
copy_no_convert(unsigned char *to, int to_stride,
                const unsigned char *from, int from_stride,
                const qpGeomVertexDataType *from_type,
                int num_records) const {
  if (to_stride == _total_bytes && from_stride == _total_bytes) {
    // Fantastic!  It's just a linear array of this one data type.
    // Copy the whole thing all at once.
    memcpy(to, from, num_records * _total_bytes);

  } else {
    // Ok, it's interleaved in with other data.  Copy them one record
    // at a time.
    while (num_records > 0) {
      memcpy(to, from, _total_bytes);
      to += to_stride;
      from += from_stride;
      num_records--;
    }
  }
}
  
////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexDataType::copy_argb_to_uint8
//       Access: Private
//  Description: Converts packed_argb to uint8-based r, g, b, a.
////////////////////////////////////////////////////////////////////
void qpGeomVertexDataType::
copy_argb_to_uint8(unsigned char *to, int to_stride,
                   const unsigned char *from, int from_stride,
                   const qpGeomVertexDataType *from_type,
                   int num_records) const {
  while (num_records > 0) {
    PN_uint32 packed_argb = *(const PN_uint32 *)from;
    to[0] = ((packed_argb >> 16) & 0xff);
    to[1] = ((packed_argb >> 8) & 0xff);
    to[2] = (packed_argb & 0xff);
    to[3] = ((packed_argb >> 24) & 0xff);

    to += to_stride;
    from += from_stride;
    num_records--;
  }
}
  
////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexDataType::copy_uint8_to_argb
//       Access: Private
//  Description: Converts uint8-based r, g, b, a to packed_argb.
////////////////////////////////////////////////////////////////////
void qpGeomVertexDataType::
copy_uint8_to_argb(unsigned char *to, int to_stride,
                   const unsigned char *from, int from_stride,
                   const qpGeomVertexDataType *from_type,
                   int num_records) const {
  while (num_records > 0) {
    PN_uint32 packed_argb = ((from[3] << 24) | (from[0] << 16) | (from[1] << 8) | from[2]);
    *(PN_uint32 *)to = packed_argb;

    to += to_stride;
    from += from_stride;
    num_records--;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexDataType::copy_generic
//       Access: Private
//  Description: A more general anything-to-anything copy (somewhat
//               more expensive than the above).
////////////////////////////////////////////////////////////////////
void qpGeomVertexDataType::
copy_generic(unsigned char *to, int to_stride,
             const unsigned char *from, int from_stride,
             const qpGeomVertexDataType *from_type,
             int num_records) const {
  int num_values_to_copy = min(get_num_values(), from_type->get_num_values());
  int num_values_to_fill = get_num_values() - num_values_to_copy;

  while (num_records > 0) {
    int vi = 0;
    while (vi < num_values_to_copy) {
      float value = from_type->get_value(from, vi);
      set_value(to, vi, value);
      ++vi;
    }
    while (vi < num_values_to_fill) {
      set_value(to, vi, 0.0f);
      ++vi;
    }

    to += to_stride;
    from += from_stride;
    num_records--;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexDataType::get_value
//       Access: Private
//  Description: Returns the nth value of the data type, expressed as
//               a float.
////////////////////////////////////////////////////////////////////
float qpGeomVertexDataType::
get_value(const unsigned char *data, int n) const {
  switch (get_numeric_type()) {
  case NT_uint16:
    return (float)data[n];

  case NT_uint8:
    return (float)data[n] / 255.0f;

  case qpGeomVertexDataType::NT_packed_8888:
    {
      int element = n / 4;
      const PN_uint32 *int_array = (const PN_uint32 *)data;

      PN_uint32 packed_argb = int_array[element];
      switch (n % 4) {
      case 0:
        return (float)((packed_argb >> 16) & 0xff) / 255.0f;
      case 1:
        return (float)((packed_argb >> 8) & 0xff) / 255.0f;
      case 2:
        return (float)(packed_argb & 0xff) / 255.0f;
      case 3:
        return (float)((packed_argb >> 24) & 0xff) / 255.0f;
      }
    }
    break;

  case qpGeomVertexDataType::NT_float32:
    {
      const PN_float32 *float_array = (const PN_float32 *)data;
      return float_array[n];
    }
  }

  return 0.0f;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexDataType::set_value
//       Access: Private
//  Description: Modifies the nth value of the data type.
////////////////////////////////////////////////////////////////////
void qpGeomVertexDataType::
set_value(unsigned char *data, int n, float value) const {
  switch (get_numeric_type()) {
  case NT_uint16:
    data[n] = (int)value;
    break;

  case NT_uint8:
    data[n] = (int)(value * 255.0f);
    break;

  case qpGeomVertexDataType::NT_packed_8888:
    {
      int element = n / 4;

      union {
        PN_uint32 _packed_argb;
        struct {
          unsigned char _a;
          unsigned char _r;
          unsigned char _g;
          unsigned char _b;
        } _argb;
      } color;

      PN_uint32 *int_array = (PN_uint32 *)data;
      color._packed_argb = int_array[element];
      switch (n % 4) {
      case 0:
        color._argb._r = (int)(value * 255.0f);
        break;
      case 1:
        color._argb._g = (int)(value * 255.0f);
        break;
      case 2:
        color._argb._b = (int)(value * 255.0f);
        break;
      case 3:
        color._argb._a = (int)(value * 255.0f);
        break;
      }
      int_array[element] = color._packed_argb;
    }
    break;

  case qpGeomVertexDataType::NT_float32:
    PN_float32 *float_array = (PN_float32 *)data;
    float_array[n] = value;
    break;
  }
}
