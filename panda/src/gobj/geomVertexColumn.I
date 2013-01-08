// Filename: geomVertexColumn.I
// Created by:  drose (06Mar05)
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
//     Function: GeomVertexColumn::Default Constructor
//       Access: Private
//  Description: Creates an invalid column.  Used only when constructing
//               from a bam file.
////////////////////////////////////////////////////////////////////
INLINE GeomVertexColumn::
GeomVertexColumn() :
  _packer(NULL)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexColumn::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE GeomVertexColumn::
GeomVertexColumn(InternalName *name, int num_components,
                 NumericType numeric_type, Contents contents,
                 int start, int column_alignment) :
  _name(name),
  _num_components(num_components),
  _numeric_type(numeric_type),
  _contents(contents),
  _start(start),
  _column_alignment(column_alignment),
  _packer(NULL)
{
  setup();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexColumn::Copy Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE GeomVertexColumn::
GeomVertexColumn(const GeomVertexColumn &copy) :
  _name(copy._name),
  _num_components(copy._num_components),
  _numeric_type(copy._numeric_type),
  _contents(copy._contents),
  _start(copy._start),
  _column_alignment(copy._column_alignment),
  _packer(NULL)
{
  setup();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexColumn::Destructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE GeomVertexColumn::
~GeomVertexColumn() {
  delete _packer;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexColumn::get_name
//       Access: Published
//  Description: Returns the name of this particular data field,
//               e.g. "vertex" or "normal".  The name may be a
//               user-defined string, or it may be one of the standard
//               system-defined field types.  Only the system-defined
//               field types are used for the actual rendering.
////////////////////////////////////////////////////////////////////
INLINE InternalName *GeomVertexColumn::
get_name() const {
  return _name;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexColumn::get_num_components
//       Access: Published
//  Description: Returns the number of components of the column:
//               the number of instances of the NumericType in each
//               element.  This is usually, but not always, the same
//               thing as get_num_values().
////////////////////////////////////////////////////////////////////
INLINE int GeomVertexColumn::
get_num_components() const {
  return _num_components;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexColumn::get_num_values
//       Access: Published
//  Description: Returns the number of numeric values of the column:
//               the number of distinct numeric values that go into
//               each element.  This is usually, but not always, the
//               same thing as get_num_components(); the difference is
//               in the case of a composite numeric type like
//               NT_packed_dcba, which has four numeric values per
//               component.
////////////////////////////////////////////////////////////////////
INLINE int GeomVertexColumn::
get_num_values() const {
  return _num_values;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexColumn::get_numeric_type
//       Access: Published
//  Description: Returns the token representing the numeric type of
//               the data storage.
////////////////////////////////////////////////////////////////////
INLINE GeomVertexColumn::NumericType GeomVertexColumn::
get_numeric_type() const {
  return _numeric_type;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexColumn::get_contents
//       Access: Published
//  Description: Returns the token representing the semantic meaning of
//               the stored value.
////////////////////////////////////////////////////////////////////
INLINE GeomVertexColumn::Contents GeomVertexColumn::
get_contents() const {
  return _contents;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexColumn::get_start
//       Access: Published
//  Description: Returns the byte within the array record at which
//               this column starts.  This can be set to non-zero
//               to implement interleaved arrays.
////////////////////////////////////////////////////////////////////
INLINE int GeomVertexColumn::
get_start() const {
  return _start;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexColumn::get_column_alignment
//       Access: Published
//  Description: Returns the alignment requirements for this column.
//               If this is greater than 1, it restricts the column to
//               appear only on memory addresses that are integer
//               multiples of this value; this has implications for
//               this column's start value, as well as the stride of
//               the resulting array.
////////////////////////////////////////////////////////////////////
INLINE int GeomVertexColumn::
get_column_alignment() const {
  return _column_alignment;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexColumn::get_component_bytes
//       Access: Published
//  Description: Returns the number of bytes used by each component
//               (that is, by one element of the numeric type).
////////////////////////////////////////////////////////////////////
INLINE int GeomVertexColumn::
get_component_bytes() const {
  return _component_bytes;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexColumn::get_total_bytes
//       Access: Published
//  Description: Returns the number of bytes used by each element of
//               the column: component_bytes * num_components.
////////////////////////////////////////////////////////////////////
INLINE int GeomVertexColumn::
get_total_bytes() const {
  return _total_bytes;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexColumn::has_homogeneous_coord
//       Access: Published
//  Description: Returns true if this Contents type is one that
//               includes a homogeneous coordinate in the fourth
//               component, or false otherwise.  If this is true,
//               correct operation on the vertex data may require
//               scaling by the homogeneous coordinate from time to
//               time (but in general this is handled automatically if
//               you use the 3-component or smaller forms of
//               get_data() and set_data()).
////////////////////////////////////////////////////////////////////
INLINE bool GeomVertexColumn::
has_homogeneous_coord() const {
  switch (_contents) {
  case C_point:
  case C_texcoord:
    return true;

  default:
    return false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexColumn::overlaps_with
//       Access: Published
//  Description: Returns true if this column overlaps with any of
//               the bytes in the indicated range, false if it does
//               not.
////////////////////////////////////////////////////////////////////
INLINE bool GeomVertexColumn::
overlaps_with(int start_byte, int num_bytes) const {
  return (_start < start_byte + num_bytes &&
          _start + _total_bytes > start_byte);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexColumn::is_bytewise_equivalent
//       Access: Published
//  Description: Returns true if the data store of this column is
//               exactly the same as that of the other, irrespective
//               of name or start position within the record.
////////////////////////////////////////////////////////////////////
INLINE bool GeomVertexColumn::
is_bytewise_equivalent(const GeomVertexColumn &other) const {
  // Not sure if the contents are relevant, but let's say that they
  // are.
  return (_num_components == other._num_components &&
          _numeric_type == other._numeric_type &&
          _contents == other._contents);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexColumn::is_packed_argb
//       Access: Public
//  Description: Returns true if this column is the standard
//               DirectX representation of 4-component color: C_color,
//               in NT_packed_dabc, with 1 component (4 values).
////////////////////////////////////////////////////////////////////
INLINE bool GeomVertexColumn::
is_packed_argb() const {
  return (_num_components == 1 &&
          _numeric_type == NT_packed_dabc &&
          _contents == C_color);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexColumn::is_uint8_rgba
//       Access: Public
//  Description: Returns true if this column is the standard
//               OpenGL representation of 4-component color: C_color,
//               in NT_uint8, with 4 components.
////////////////////////////////////////////////////////////////////
INLINE bool GeomVertexColumn::
is_uint8_rgba() const {
  return (_num_components == 4 &&
          _numeric_type == NT_uint8 &&
          _contents == C_color);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexColumn::compare_to
//       Access: Public
//  Description: This is used to unquify columns, and hence
//               formats, for the GeomVertexFormat registry.
////////////////////////////////////////////////////////////////////
INLINE int GeomVertexColumn::
compare_to(const GeomVertexColumn &other) const {
  if (_name != other._name) {
    return _name < other._name ? -1 : 1;
  }
  if (_num_components != other._num_components) {
    return _num_components - other._num_components;
  }
  if (_numeric_type != other._numeric_type) {
    return (int)_numeric_type - (int)other._numeric_type;
  }
  if (_contents != other._contents) {
    return (int)_contents - (int)other._contents;
  }
  if (_start != other._start) {
    return _start - other._start;
  }
  if (_column_alignment != other._column_alignment) {
    return _column_alignment - other._column_alignment;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexColumn::operator ==
//       Access: Public
//  Description: Returns true if the two columns are exactly
//               equivalent, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool GeomVertexColumn::
operator == (const GeomVertexColumn &other) const {
  return compare_to(other) == 0;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexColumn::operator !=
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool GeomVertexColumn::
operator != (const GeomVertexColumn &other) const {
  return compare_to(other) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexColumn::operator <
//       Access: Public
//  Description: This is used to put columns in order within a
//               particular GeomVertexArrayFormat.  Note that it is
//               *not* in the same space as operator == and operator
//               !=.
////////////////////////////////////////////////////////////////////
INLINE bool GeomVertexColumn::
operator < (const GeomVertexColumn &other) const {
  if (_start != other._start) { 
    return _start < other._start;
  }
  if (_total_bytes < other._total_bytes) {
    return _total_bytes < other._total_bytes;
  }
  return 0;
}

INLINE ostream &
operator << (ostream &out, const GeomVertexColumn &obj) {
  obj.output(out);
  return out;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexColumn::Packer::maybe_scale_color_f
//       Access: Public
//  Description: Converts an integer (typically a uint8) value to a
//               floating-point value.  If the contents value
//               indicates this is a color value, scales it into the
//               range 0..1 per convention; otherwise leaves it alone.
////////////////////////////////////////////////////////////////////
INLINE float GeomVertexColumn::Packer::
maybe_scale_color_f(unsigned int value) {
  if (_column->get_contents() == C_color) {
    return (float)value / 255.0f;
  } else {
    return (float)value;
  }
}
    
////////////////////////////////////////////////////////////////////
//     Function: GeomVertexColumn::Packer::maybe_scale_color_f
//       Access: Public
//  Description: Converts a pair of integers into the _v2 member.  See
//               one-parameter maybe_scale_color_f() for more info.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexColumn::Packer::
maybe_scale_color_f(unsigned int a, unsigned int b) {
  if (_column->get_contents() == C_color) {
    _v2.set((float)a / 255.0f,
            (float)b / 255.0f);
  } else {    
    _v2.set((float)a, (float)b);
  }
}
    
////////////////////////////////////////////////////////////////////
//     Function: GeomVertexColumn::Packer::maybe_scale_color_f
//       Access: Public
//  Description: Converts a pair of integers into the _v3 member.  See
//               one-parameter maybe_scale_color_f() for more info.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexColumn::Packer::
maybe_scale_color_f(unsigned int a, unsigned int b, unsigned int c) {
  if (_column->get_contents() == C_color) {
    _v3.set((float)a / 255.0f,
            (float)b / 255.0f,
            (float)c / 255.0f);
  } else {    
    _v3.set((float)a, (float)b, (float)c);
  }
}
    
////////////////////////////////////////////////////////////////////
//     Function: GeomVertexColumn::Packer::maybe_scale_color_f
//       Access: Public
//  Description: Converts a pair of integers into the _v4 member.  See
//               one-parameter maybe_scale_color_f() for more info.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexColumn::Packer::
maybe_scale_color_f(unsigned int a, unsigned int b, unsigned int c,
                    unsigned int d) {
  if (_column->get_contents() == C_color) {
    _v4.set((float)a / 255.0f,
            (float)b / 255.0f,
            (float)c / 255.0f,
            (float)d / 255.0f);
  } else {
    _v4.set((float)a, (float)b, (float)c, (float)d);
  }
}
    

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexColumn::Packer::maybe_unscale_color_f
//       Access: Public
//  Description: Converts a floating-point value to a uint8 value.  If
//               the contents value indicates this is a color value,
//               scales it into the range 0..255 per convention;
//               otherwise leaves it alone.
////////////////////////////////////////////////////////////////////
INLINE unsigned int GeomVertexColumn::Packer::
maybe_unscale_color_f(float data) {
  if (_column->get_contents() == C_color) {
    return (unsigned int)(data * 255.0f);
  } else {
    return (unsigned int)data;
  }
}
    
////////////////////////////////////////////////////////////////////
//     Function: GeomVertexColumn::Packer::maybe_unscale_color_f
//       Access: Public
//  Description: Converts an LVecBase2f into a pair of uint8
//               values.  See one-parameter maybe_unscale_color_f() for
//               more info.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexColumn::Packer::
maybe_unscale_color_f(const LVecBase2f &data) {
  if (_column->get_contents() == C_color) {
    _a = (unsigned int)(data[0] * 255.0f);
    _b = (unsigned int)(data[1] * 255.0f);
  } else {
    _a = (unsigned int)data[0];
    _b = (unsigned int)data[1];
  }
}
    
////////////////////////////////////////////////////////////////////
//     Function: GeomVertexColumn::Packer::maybe_unscale_color_f
//       Access: Public
//  Description: Converts an LVecBase3f into a pair of uint8
//               values.  See one-parameter maybe_unscale_color_f() for
//               more info.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexColumn::Packer::
maybe_unscale_color_f(const LVecBase3f &data) {
  if (_column->get_contents() == C_color) {
    _a = (unsigned int)(data[0] * 255.0f);
    _b = (unsigned int)(data[1] * 255.0f);
    _c = (unsigned int)(data[2] * 255.0f);
  } else {
    _a = (unsigned int)data[0];
    _b = (unsigned int)data[1];
    _c = (unsigned int)data[2];
  }
}
    
////////////////////////////////////////////////////////////////////
//     Function: GeomVertexColumn::Packer::maybe_unscale_color_f
//       Access: Public
//  Description: Converts an LVecBase4f into a pair of uint8
//               values.  See one-parameter maybe_unscale_color_f() for
//               more info.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexColumn::Packer::
maybe_unscale_color_f(const LVecBase4f &data) {
  if (_column->get_contents() == C_color) {
    _a = (unsigned int)(data[0] * 255.0f);
    _b = (unsigned int)(data[1] * 255.0f);
    _c = (unsigned int)(data[2] * 255.0f);
    _d = (unsigned int)(data[3] * 255.0f);
  } else {
    _a = (unsigned int)data[0];
    _b = (unsigned int)data[1];
    _c = (unsigned int)data[2];
    _d = (unsigned int)data[3];
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexColumn::Packer::maybe_scale_color_d
//       Access: Public
//  Description: Converts an integer (typically a uint8) value to a
//               floating-point value.  If the contents value
//               indicates this is a color value, scales it into the
//               range 0..1 per convention; otherwise leaves it alone.
////////////////////////////////////////////////////////////////////
INLINE double GeomVertexColumn::Packer::
maybe_scale_color_d(unsigned int value) {
  if (_column->get_contents() == C_color) {
    return (double)value / 255.0;
  } else {
    return (double)value;
  }
}
    
////////////////////////////////////////////////////////////////////
//     Function: GeomVertexColumn::Packer::maybe_scale_color_d
//       Access: Public
//  Description: Converts a pair of integers into the _v2d member.  See
//               one-parameter maybe_scale_color_d() for more info.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexColumn::Packer::
maybe_scale_color_d(unsigned int a, unsigned int b) {
  if (_column->get_contents() == C_color) {
    _v2d.set((double)a / 255.0,
             (double)b / 255.0);
  } else {    
    _v2d.set((double)a, (double)b);
  }
}
    
////////////////////////////////////////////////////////////////////
//     Function: GeomVertexColumn::Packer::maybe_scale_color_d
//       Access: Public
//  Description: Converts a pair of integers into the _v3d member.  See
//               one-parameter maybe_scale_color_d() for more info.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexColumn::Packer::
maybe_scale_color_d(unsigned int a, unsigned int b, unsigned int c) {
  if (_column->get_contents() == C_color) {
    _v3d.set((double)a / 255.0,
             (double)b / 255.0,
             (double)c / 255.0);
  } else {    
    _v3d.set((double)a, (double)b, (double)c);
  }
}
    
////////////////////////////////////////////////////////////////////
//     Function: GeomVertexColumn::Packer::maybe_scale_color_d
//       Access: Public
//  Description: Converts a pair of integers into the _v4d member.  See
//               one-parameter maybe_scale_color_d() for more info.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexColumn::Packer::
maybe_scale_color_d(unsigned int a, unsigned int b, unsigned int c,
                    unsigned int d) {
  if (_column->get_contents() == C_color) {
    _v4d.set((double)a / 255.0,
             (double)b / 255.0,
             (double)c / 255.0,
             (double)d / 255.0);
  } else {
    _v4d.set((double)a, (double)b, (double)c, (double)d);
  }
}
    

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexColumn::Packer::maybe_unscale_color_d
//       Access: Public
//  Description: Converts a floating-point value to a uint8 value.  If
//               the contents value indicates this is a color value,
//               scales it into the range 0..255 per convention;
//               otherwise leaves it alone.
////////////////////////////////////////////////////////////////////
INLINE unsigned int GeomVertexColumn::Packer::
maybe_unscale_color_d(double data) {
  if (_column->get_contents() == C_color) {
    return (unsigned int)(data * 255.0);
  } else {
    return (unsigned int)data;
  }
}
    
////////////////////////////////////////////////////////////////////
//     Function: GeomVertexColumn::Packer::maybe_unscale_color_d
//       Access: Public
//  Description: Converts an LVecBase2d into a pair of uint8
//               values.  See one-parameter maybe_unscale_color_d() for
//               more info.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexColumn::Packer::
maybe_unscale_color_d(const LVecBase2d &data) {
  if (_column->get_contents() == C_color) {
    _a = (unsigned int)(data[0] * 255.0);
    _b = (unsigned int)(data[1] * 255.0);
  } else {
    _a = (unsigned int)data[0];
    _b = (unsigned int)data[1];
  }
}
    
////////////////////////////////////////////////////////////////////
//     Function: GeomVertexColumn::Packer::maybe_unscale_color_d
//       Access: Public
//  Description: Converts an LVecBase3d into a pair of uint8
//               values.  See one-parameter maybe_unscale_color_d() for
//               more info.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexColumn::Packer::
maybe_unscale_color_d(const LVecBase3d &data) {
  if (_column->get_contents() == C_color) {
    _a = (unsigned int)(data[0] * 255.0);
    _b = (unsigned int)(data[1] * 255.0);
    _c = (unsigned int)(data[2] * 255.0);
  } else {
    _a = (unsigned int)data[0];
    _b = (unsigned int)data[1];
    _c = (unsigned int)data[2];
  }
}
    
////////////////////////////////////////////////////////////////////
//     Function: GeomVertexColumn::Packer::maybe_unscale_color_d
//       Access: Public
//  Description: Converts an LVecBase4d into a pair of uint8
//               values.  See one-parameter maybe_unscale_color_d() for
//               more info.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexColumn::Packer::
maybe_unscale_color_d(const LVecBase4d &data) {
  if (_column->get_contents() == C_color) {
    _a = (unsigned int)(data[0] * 255.0);
    _b = (unsigned int)(data[1] * 255.0);
    _c = (unsigned int)(data[2] * 255.0);
    _d = (unsigned int)(data[3] * 255.0);
  } else {
    _a = (unsigned int)data[0];
    _b = (unsigned int)data[1];
    _c = (unsigned int)data[2];
    _d = (unsigned int)data[3];
  }
}
