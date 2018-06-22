/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomVertexColumn.I
 * @author drose
 * @date 2005-03-06
 */

/**
 * Creates an invalid column.  Used only when constructing from a bam file.
 */
INLINE GeomVertexColumn::
GeomVertexColumn() :
  _packer(nullptr)
{
}

/**
 *
 */
INLINE GeomVertexColumn::
GeomVertexColumn(CPT_InternalName name, int num_components,
                 NumericType numeric_type, Contents contents,
                 int start, int column_alignment, int num_elements,
                 int element_stride) :
  _name(std::move(name)),
  _num_components(num_components),
  _numeric_type(numeric_type),
  _contents(contents),
  _start(start),
  _column_alignment(column_alignment),
  _num_elements(num_elements),
  _element_stride(element_stride),
  _packer(nullptr)
{
  setup();
}

/**
 *
 */
INLINE GeomVertexColumn::
GeomVertexColumn(const GeomVertexColumn &copy) :
  _name(copy._name),
  _num_components(copy._num_components),
  _numeric_type(copy._numeric_type),
  _contents(copy._contents),
  _start(copy._start),
  _column_alignment(copy._column_alignment),
  _num_elements(copy._num_elements),
  _element_stride(copy._element_stride),
  _packer(nullptr)
{
  setup();
}

/**
 *
 */
INLINE GeomVertexColumn::
~GeomVertexColumn() {
  delete _packer;
}

/**
 * Returns the name of this particular data field, e.g.  "vertex" or "normal".
 * The name may be a user-defined string, or it may be one of the standard
 * system-defined field types.  Only the system-defined field types are used
 * for the actual rendering.
 */
INLINE const InternalName *GeomVertexColumn::
get_name() const {
  return _name;
}

/**
 * Returns the number of components of the column: the number of instances of
 * the NumericType in each element.  This is usually, but not always, the same
 * thing as get_num_values().
 */
INLINE int GeomVertexColumn::
get_num_components() const {
  return _num_components;
}

/**
 * Returns the number of numeric values of the column: the number of distinct
 * numeric values that go into each element.  This is usually, but not always,
 * the same thing as get_num_components(); the difference is in the case of a
 * composite numeric type like NT_packed_dcba, which has four numeric values
 * per component.
 */
INLINE int GeomVertexColumn::
get_num_values() const {
  return _num_values;
}

/**
 * Returns the number of times this column is repeated.  This is usually 1,
 * except for matrices.
 */
INLINE int GeomVertexColumn::
get_num_elements() const {
  return _num_elements;
}

/**
 * Returns the token representing the numeric type of the data storage.
 */
INLINE GeomVertexColumn::NumericType GeomVertexColumn::
get_numeric_type() const {
  return _numeric_type;
}

/**
 * Returns the token representing the semantic meaning of the stored value.
 */
INLINE GeomVertexColumn::Contents GeomVertexColumn::
get_contents() const {
  return _contents;
}

/**
 * Returns the byte within the array record at which this column starts.  This
 * can be set to non-zero to implement interleaved arrays.
 */
INLINE int GeomVertexColumn::
get_start() const {
  return _start;
}

/**
 * Returns the alignment requirements for this column.  If this is greater
 * than 1, it restricts the column to appear only on memory addresses that are
 * integer multiples of this value; this has implications for this column's
 * start value, as well as the stride of the resulting array.
 */
INLINE int GeomVertexColumn::
get_column_alignment() const {
  return _column_alignment;
}

/**
 * This value is only relevant for matrix types.  Returns the number of bytes
 * to add to access the next row of the matrix.
 */
INLINE int GeomVertexColumn::
get_element_stride() const {
  return _element_stride;
}

/**
 * Returns the number of bytes used by each component (that is, by one element
 * of the numeric type).
 */
INLINE int GeomVertexColumn::
get_component_bytes() const {
  return _component_bytes;
}

/**
 * Returns the number of bytes used by each element of the column:
 * component_bytes * num_components.
 */
INLINE int GeomVertexColumn::
get_total_bytes() const {
  return _total_bytes;
}

/**
 * Returns true if this Contents type is one that includes a homogeneous
 * coordinate in the fourth component, or false otherwise.  If this is true,
 * correct operation on the vertex data may require scaling by the homogeneous
 * coordinate from time to time (but in general this is handled automatically
 * if you use the 3-component or smaller forms of get_data() and set_data()).
 */
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

/**
 * Returns true if this column overlaps with any of the bytes in the indicated
 * range, false if it does not.
 */
INLINE bool GeomVertexColumn::
overlaps_with(int start_byte, int num_bytes) const {
  return (_start < start_byte + num_bytes &&
          _start + _total_bytes > start_byte);
}

/**
 * Returns true if the data store of this column is exactly the same as that
 * of the other, irrespective of name or start position within the record.
 */
INLINE bool GeomVertexColumn::
is_bytewise_equivalent(const GeomVertexColumn &other) const {
  // Not sure if the contents are relevant, but let's say that they are.
  return (_num_components == other._num_components &&
          _numeric_type == other._numeric_type &&
          _contents == other._contents);
}

/**
 * Returns true if this column is the standard DirectX representation of
 * 4-component color: C_color, in NT_packed_dabc, with 1 component (4 values).
 */
INLINE bool GeomVertexColumn::
is_packed_argb() const {
  return (_num_components == 1 &&
          _numeric_type == NT_packed_dabc &&
          _contents == C_color);
}

/**
 * Returns true if this column is the standard OpenGL representation of
 * 4-component color: C_color, in NT_uint8, with 4 components.
 */
INLINE bool GeomVertexColumn::
is_uint8_rgba() const {
  return (_num_components == 4 &&
          _numeric_type == NT_uint8 &&
          _contents == C_color);
}

/**
 * This is used to unquify columns, and hence formats, for the
 * GeomVertexFormat registry.
 */
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
  if (_num_elements != other._num_elements) {
    return _num_elements - other._num_elements;
  }
  if (_element_stride != other._element_stride) {
    return _element_stride - other._element_stride;
  }
  return 0;
}

/**
 * Returns true if the two columns are exactly equivalent, false otherwise.
 */
INLINE bool GeomVertexColumn::
operator == (const GeomVertexColumn &other) const {
  return compare_to(other) == 0;
}

/**
 *
 */
INLINE bool GeomVertexColumn::
operator != (const GeomVertexColumn &other) const {
  return compare_to(other) != 0;
}

/**
 * This is used to put columns in order within a particular
 * GeomVertexArrayFormat.  Note that it is *not* in the same space as operator
 * == and operator !=.
 */
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

INLINE std::ostream &
operator << (std::ostream &out, const GeomVertexColumn &obj) {
  obj.output(out);
  return out;
}
