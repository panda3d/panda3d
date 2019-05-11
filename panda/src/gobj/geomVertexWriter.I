/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomVertexWriter.I
 * @author drose
 * @date 2005-03-25
 */

/**
 * Constructs an invalid GeomVertexWriter.  You must use the assignment
 * operator to assign a valid GeomVertexWriter to this object before you can
 * use it.
 */
INLINE GeomVertexWriter::
GeomVertexWriter(Thread *current_thread) :
  _vertex_data(nullptr),
  _current_thread(current_thread)
{
  initialize();
}

/**
 * Constructs a new writer to process the vertices of the indicated data
 * object.
 */
INLINE GeomVertexWriter::
GeomVertexWriter(GeomVertexData *vertex_data, Thread *current_thread) :
  _vertex_data(vertex_data),
  _current_thread(current_thread)
{
  initialize();
}

/**
 * Constructs a new writer to process the vertices of the indicated data
 * object.  This flavor creates the writer specifically to process the named
 * data type.
 */
INLINE GeomVertexWriter::
GeomVertexWriter(GeomVertexData *vertex_data, CPT_InternalName name,
                 Thread *current_thread) :
  _vertex_data(vertex_data),
  _current_thread(current_thread)
{
  initialize();
  set_column(std::move(name));
}

/**
 * Constructs a new writer to process the vertices of the indicated array
 * only.
 */
INLINE GeomVertexWriter::
GeomVertexWriter(GeomVertexArrayData *array_data,
                 Thread *current_thread) :
  _array_data(array_data),
  _current_thread(current_thread)
{
  initialize();
}

/**
 * Constructs a new writer to process the vertices of the indicated array
 * only.
 */
INLINE GeomVertexWriter::
GeomVertexWriter(GeomVertexArrayData *array_data, int column,
                 Thread *current_thread) :
  _array_data(array_data),
  _current_thread(current_thread)
{
  initialize();
  set_column(column);
}

/**
 * Constructs a new writer to process the vertices of the indicated data
 * object.  This flavor creates the writer specifically to process the named
 * data type.
 */
INLINE GeomVertexWriter::
GeomVertexWriter(GeomVertexDataPipelineWriter *data_writer,
                 const InternalName *name) :
  _vertex_data(data_writer->get_object()),
  _current_thread(data_writer->get_current_thread())
{
  initialize();
  const GeomVertexFormat *format = data_writer->get_format();
  set_vertex_column(format->get_array_with(name),
                    format->get_column(name),
                    data_writer);
}

/**
 *
 */
INLINE GeomVertexWriter::
GeomVertexWriter(const GeomVertexWriter &copy) :
  _vertex_data(copy._vertex_data),
  _array(copy._array),
  _array_data(copy._array_data),
  _current_thread(copy._current_thread),
  _packer(copy._packer),
  _stride(copy._stride),
  _handle(copy._handle),
  _pointer_begin(copy._pointer_begin),
  _pointer_end(copy._pointer_end),
  _pointer(copy._pointer),
  _start_row(copy._start_row)
{
}

/**
 *
 */
INLINE void GeomVertexWriter::
operator = (const GeomVertexWriter &copy) {
  _vertex_data = copy._vertex_data;
  _array = copy._array;
  _array_data = copy._array_data;
  _current_thread = copy._current_thread;
  _packer = copy._packer;
  _stride = copy._stride;
  _handle = copy._handle;
  _pointer_begin = copy._pointer_begin;
  _pointer_end = copy._pointer_end;
  _pointer = copy._pointer;
  _start_row = copy._start_row;
}

/**
 *
 */
INLINE GeomVertexWriter::
~GeomVertexWriter() {
}

/**
 * Returns the vertex data object that the writer is processing.  This may
 * return NULL if the writer was constructed with just an array pointer.
 */
INLINE GeomVertexData *GeomVertexWriter::
get_vertex_data() const {
  return _vertex_data;
}

/**
 * Returns the particular array object that the writer is currently
 * processing.
 */
INLINE GeomVertexArrayData *GeomVertexWriter::
get_array_data() const {
  return _array_data;
}

/**
 * Returns the write handle to the array object that the writer is currently
 * processing.  This low-level call should be used with caution; be careful
 * with modifying the data in the handle out from under the GeomVertexWriter.
 */
INLINE GeomVertexArrayDataHandle *GeomVertexWriter::
get_array_handle() const {
  return _handle;
}

/**
 * Returns the per-row stride (bytes between consecutive rows) of the
 * underlying vertex array.  This low-level information is normally not needed
 * to use the GeomVertexWriter directly.
 */
INLINE size_t GeomVertexWriter::
get_stride() const {
  return _stride;
}

/**
 * Returns the Thread pointer of the currently-executing thread, as passed to
 * the constructor of this object.
 */
INLINE Thread *GeomVertexWriter::
get_current_thread() const {
  return _current_thread;
}

/**
 * Sets up the writer to use the nth data type of the GeomVertexFormat,
 * numbering from 0.
 *
 * This also resets the write row number to the start row (the same value
 * passed to a previous call to set_row(), or 0 if set_row() was never
 * called.)
 *
 * The return value is true if the data type is valid, false otherwise.
 */
INLINE bool GeomVertexWriter::
set_column(int column) {
  if (_vertex_data != nullptr) {
    GeomVertexDataPipelineWriter writer(_vertex_data, true, _current_thread);
    writer.check_array_writers();
    const GeomVertexFormat *format = writer.get_format();
    return set_vertex_column(format->get_array_with(column),
                             format->get_column(column),
                             &writer);
  }
  if (_array_data != nullptr) {
    return set_array_column(_array_data->get_array_format()->get_column(column));
  }
  return false;
}

/**
 * Sets up the writer to use the data type with the indicated name.
 *
 * This also resets the write number to the start row (the same value passed
 * to a previous call to set_row(), or 0 if set_row() was never called.)
 *
 * The return value is true if the data type is valid, false otherwise.
 */
INLINE bool GeomVertexWriter::
set_column(CPT_InternalName name) {
  if (_vertex_data != nullptr) {
    GeomVertexDataPipelineWriter writer(_vertex_data, true, _current_thread);
    writer.check_array_writers();
    const GeomVertexFormat *format = writer.get_format();
    return set_vertex_column(format->get_array_with(name),
                             format->get_column(name),
                             &writer);
  }
  if (_array_data != nullptr) {
    return set_array_column(_array_data->get_array_format()->get_column(name));
  }
  return false;
}

/**
 * Resets the GeomVertexWriter to the initial state.
 */
INLINE void GeomVertexWriter::
clear() {
  (*this) = GeomVertexWriter(_current_thread);
}

/**
 * Returns true if a valid data type has been successfully set, or false if
 * the data type does not exist.
 */
INLINE bool GeomVertexWriter::
has_column() const {
  return (_packer != nullptr);
}

/**
 * Returns the array index containing the data type that the writer is working
 * on.
 */
INLINE int GeomVertexWriter::
get_array() const {
  return _array;
}

/**
 * Returns the description of the data type that the writer is working on.
 */
INLINE const GeomVertexColumn *GeomVertexWriter::
get_column() const {
  if (_packer != nullptr) {
    return _packer->_column;
  }
  return nullptr;
}

/**
 * Sets the start row to the indicated value, without internal checks.  This
 * is the same as set_row(), but it does not check for the possibility that
 * the array has been reallocated internally for some reason; use only when
 * you are confident that the array is unchanged and you really need every bit
 * of available performance.
 */
INLINE void GeomVertexWriter::
set_row_unsafe(int row) {
  _start_row = row;
  if (has_column()) {
    quick_set_pointer(_start_row);
  }
}

/**
 * Sets the start row to the indicated value.  The writer will begin writing
 * to the indicated row; each subsequent set_data*() call will store the data
 * into the subsequent row.  If set_column() is called, the writer will return
 * to this row.
 */
INLINE void GeomVertexWriter::
set_row(int row) {
  nassertv(row >= 0);
  _start_row = row;
  if (has_column()) {
    set_pointer(_start_row);
  }
}

/**
 * Returns the row index at which the writer started.  It will return to this
 * row if you reset the current column.
 */
INLINE int GeomVertexWriter::
get_start_row() const {
  return _start_row;
}

/**
 * Returns the row index to which the data will be written at the next call to
 * set_data*() or add_data*().
 */
INLINE int GeomVertexWriter::
get_write_row() const {
  return (int)(_pointer - _pointer_begin) / _stride;
}

/**
 * Returns true if the writer is currently at the end of the list of vertices,
 * false otherwise.  If this is true, another call to set_data*() will result
 * in a crash, but another call to add_data*() will add a new row.
 */
INLINE bool GeomVertexWriter::
is_at_end() const {
  return _pointer >= _pointer_end;
}

/**
 * Sets the write row to a particular 1-component value, and advances the
 * write row.
 *
 * It is an error for the write row to advance past the end of data.
 */
INLINE void GeomVertexWriter::
set_data1f(float data) {
  nassertv(has_column());
  _packer->set_data1f(inc_pointer(), data);
}

/**
 * Sets the write row to a particular 2-component value, and advances the
 * write row.
 *
 * It is an error for the write row to advance past the end of data.
 */
INLINE void GeomVertexWriter::
set_data2f(float x, float y) {
  set_data2f(LVecBase2f(x, y));
}

/**
 * Sets the write row to a particular 2-component value, and advances the
 * write row.
 *
 * It is an error for the write row to advance past the end of data.
 */
INLINE void GeomVertexWriter::
set_data2f(const LVecBase2f &data) {
  nassertv(has_column());
  _packer->set_data2f(inc_pointer(), data);
}

/**
 * Sets the write row to a particular 3-component value, and advances the
 * write row.
 *
 * It is an error for the write row to advance past the end of data.
 */
INLINE void GeomVertexWriter::
set_data3f(float x, float y, float z) {
  set_data3f(LVecBase3f(x, y, z));
}

/**
 * Sets the write row to a particular 3-component value, and advances the
 * write row.
 *
 * It is an error for the write row to advance past the end of data.
 */
INLINE void GeomVertexWriter::
set_data3f(const LVecBase3f &data) {
  nassertv(has_column());
  _packer->set_data3f(inc_pointer(), data);
}

/**
 * Sets the write row to a particular 4-component value, and advances the
 * write row.
 *
 * It is an error for the write row to advance past the end of data.
 */
INLINE void GeomVertexWriter::
set_data4f(float x, float y, float z, float w) {
  set_data4f(LVecBase4f(x, y, z, w));
}

/**
 * Sets the write row to a particular 4-component value, and advances the
 * write row.
 *
 * It is an error for the write row to advance past the end of data.
 */
INLINE void GeomVertexWriter::
set_data4f(const LVecBase4f &data) {
  nassertv(has_column());
  _packer->set_data4f(inc_pointer(), data);
}

/**
 * Sets the write row to a 3-by-3 matrix, and advances the write row.  This is
 * a special method that can only be used on matrix columns.
 *
 * It is an error for the write row to advance past the end of data.
 */
INLINE void GeomVertexWriter::
set_matrix3f(const LMatrix3f &mat) {
  nassertv(has_column() &&
           _packer->_column->get_contents() == C_matrix &&
           _packer->_column->get_num_elements() == 3);

  size_t col_stride = _packer->_column->get_element_stride();
  unsigned char *pointer = inc_pointer();

  _packer->set_data3f(pointer, mat.get_row(0));
  pointer += col_stride;
  _packer->set_data3f(pointer, mat.get_row(1));
  pointer += col_stride;
  _packer->set_data3f(pointer, mat.get_row(2));
}

/**
 * Sets the write row to a 4-by-4 matrix, and advances the write row.  This is
 * a special method that can only be used on matrix columns.
 *
 * It is an error for the write row to advance past the end of data.
 */
INLINE void GeomVertexWriter::
set_matrix4f(const LMatrix4f &mat) {
  nassertv(has_column() &&
           _packer->_column->get_contents() == C_matrix &&
           _packer->_column->get_num_elements() == 4);

  size_t col_stride = _packer->_column->get_element_stride();
  unsigned char *pointer = inc_pointer();

  _packer->set_data4f(pointer, mat.get_row(0));
  pointer += col_stride;
  _packer->set_data4f(pointer, mat.get_row(1));
  pointer += col_stride;
  _packer->set_data4f(pointer, mat.get_row(2));
  pointer += col_stride;
  _packer->set_data4f(pointer, mat.get_row(3));
}

/**
 * Sets the write row to a particular 1-component value, and advances the
 * write row.
 *
 * It is an error for the write row to advance past the end of data.
 */
INLINE void GeomVertexWriter::
set_data1d(double data) {
  nassertv(has_column());
  _packer->set_data1d(inc_pointer(), data);
}

/**
 * Sets the write row to a particular 2-component value, and advances the
 * write row.
 *
 * It is an error for the write row to advance past the end of data.
 */
INLINE void GeomVertexWriter::
set_data2d(double x, double y) {
  set_data2d(LVecBase2d(x, y));
}

/**
 * Sets the write row to a particular 2-component value, and advances the
 * write row.
 *
 * It is an error for the write row to advance past the end of data.
 */
INLINE void GeomVertexWriter::
set_data2d(const LVecBase2d &data) {
  nassertv(has_column());
  _packer->set_data2d(inc_pointer(), data);
}

/**
 * Sets the write row to a particular 3-component value, and advances the
 * write row.
 *
 * It is an error for the write row to advance past the end of data.
 */
INLINE void GeomVertexWriter::
set_data3d(double x, double y, double z) {
  set_data3d(LVecBase3d(x, y, z));
}

/**
 * Sets the write row to a particular 3-component value, and advances the
 * write row.
 *
 * It is an error for the write row to advance past the end of data.
 */
INLINE void GeomVertexWriter::
set_data3d(const LVecBase3d &data) {
  nassertv(has_column());
  _packer->set_data3d(inc_pointer(), data);
}

/**
 * Sets the write row to a particular 4-component value, and advances the
 * write row.
 *
 * It is an error for the write row to advance past the end of data.
 */
INLINE void GeomVertexWriter::
set_data4d(double x, double y, double z, double w) {
  set_data4d(LVecBase4d(x, y, z, w));
}

/**
 * Sets the write row to a particular 4-component value, and advances the
 * write row.
 *
 * It is an error for the write row to advance past the end of data.
 */
INLINE void GeomVertexWriter::
set_data4d(const LVecBase4d &data) {
  nassertv(has_column());
  _packer->set_data4d(inc_pointer(), data);
}

/**
 * Sets the write row to a 3-by-3 matrix, and advances the write row.  This is
 * a special method that can only be used on matrix columns.
 *
 * It is an error for the write row to advance past the end of data.
 */
INLINE void GeomVertexWriter::
set_matrix3d(const LMatrix3d &mat) {
  nassertv(has_column() &&
           _packer->_column->get_contents() == C_matrix &&
           _packer->_column->get_num_elements() == 3);

  size_t col_stride = _packer->_column->get_element_stride();
  unsigned char *pointer = inc_pointer();

  _packer->set_data3d(pointer, mat.get_row(0));
  pointer += col_stride;
  _packer->set_data3d(pointer, mat.get_row(1));
  pointer += col_stride;
  _packer->set_data3d(pointer, mat.get_row(2));
}

/**
 * Sets the write row to a 4-by-4 matrix, and advances the write row.  This is
 * a special method that can only be used on matrix columns.
 *
 * It is an error for the write row to advance past the end of data.
 */
INLINE void GeomVertexWriter::
set_matrix4d(const LMatrix4d &mat) {
  nassertv(has_column() &&
           _packer->_column->get_contents() == C_matrix &&
           _packer->_column->get_num_elements() == 4);

  size_t col_stride = _packer->_column->get_element_stride();
  unsigned char *pointer = inc_pointer();

  _packer->set_data4d(pointer, mat.get_row(0));
  pointer += col_stride;
  _packer->set_data4d(pointer, mat.get_row(1));
  pointer += col_stride;
  _packer->set_data4d(pointer, mat.get_row(2));
  pointer += col_stride;
  _packer->set_data4d(pointer, mat.get_row(3));
}

/**
 * Sets the write row to a particular 1-component value, and advances the
 * write row.
 *
 * It is an error for the write row to advance past the end of data.
 */
INLINE void GeomVertexWriter::
set_data1(PN_stdfloat data) {
#ifndef STDFLOAT_DOUBLE
  set_data1f(data);
#else
  set_data1d(data);
#endif
}

/**
 * Sets the write row to a particular 2-component value, and advances the
 * write row.
 *
 * It is an error for the write row to advance past the end of data.
 */
INLINE void GeomVertexWriter::
set_data2(PN_stdfloat x, PN_stdfloat y) {
#ifndef STDFLOAT_DOUBLE
  set_data2f(x, y);
#else
  set_data2d(x, y);
#endif
}

/**
 * Sets the write row to a particular 2-component value, and advances the
 * write row.
 *
 * It is an error for the write row to advance past the end of data.
 */
INLINE void GeomVertexWriter::
set_data2(const LVecBase2 &data) {
#ifndef STDFLOAT_DOUBLE
  set_data2f(data);
#else
  set_data2d(data);
#endif
}

/**
 * Sets the write row to a particular 3-component value, and advances the
 * write row.
 *
 * It is an error for the write row to advance past the end of data.
 */
INLINE void GeomVertexWriter::
set_data3(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z) {
#ifndef STDFLOAT_DOUBLE
  set_data3f(x, y, z);
#else
  set_data3d(x, y, z);
#endif
}

/**
 * Sets the write row to a particular 3-component value, and advances the
 * write row.
 *
 * It is an error for the write row to advance past the end of data.
 */
INLINE void GeomVertexWriter::
set_data3(const LVecBase3 &data) {
#ifndef STDFLOAT_DOUBLE
  set_data3f(data);
#else
  set_data3d(data);
#endif
}

/**
 * Sets the write row to a particular 4-component value, and advances the
 * write row.
 *
 * It is an error for the write row to advance past the end of data.
 */
INLINE void GeomVertexWriter::
set_data4(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z, PN_stdfloat w) {
#ifndef STDFLOAT_DOUBLE
  set_data4f(x, y, z, w);
#else
  set_data4d(x, y, z, w);
#endif
}

/**
 * Sets the write row to a particular 4-component value, and advances the
 * write row.
 *
 * It is an error for the write row to advance past the end of data.
 */
INLINE void GeomVertexWriter::
set_data4(const LVecBase4 &data) {
#ifndef STDFLOAT_DOUBLE
  set_data4f(data);
#else
  set_data4d(data);
#endif
}

/**
 * Sets the write row to a 3-by-3 matrix, and advances the write row.  This is
 * a special method that can only be used on matrix columns.
 *
 * It is an error for the write row to advance past the end of data.
 */
INLINE void GeomVertexWriter::
set_matrix3(const LMatrix3 &mat) {
#ifndef STDFLOAT_DOUBLE
  set_matrix3f(mat);
#else
  set_matrix3d(mat);
#endif
}

/**
 * Sets the write row to a 4-by-4 matrix, and advances the write row.  This is
 * a special method that can only be used on matrix columns.
 *
 * It is an error for the write row to advance past the end of data.
 */
INLINE void GeomVertexWriter::
set_matrix4(const LMatrix4 &mat) {
#ifndef STDFLOAT_DOUBLE
  set_matrix4f(mat);
#else
  set_matrix4d(mat);
#endif
}

/**
 * Sets the write row to a particular 1-component value, and advances the
 * write row.
 *
 * It is an error for the write row to advance past the end of data.
 */
INLINE void GeomVertexWriter::
set_data1i(int data) {
  nassertv(has_column());
  _packer->set_data1i(inc_pointer(), data);
}

/**
 * Sets the write row to a particular 2-component value, and advances the
 * write row.
 *
 * It is an error for the write row to advance past the end of data.
 */
INLINE void GeomVertexWriter::
set_data2i(int a, int b) {
  set_data2i(LVecBase2i(a, b));
}

/**
 * Sets the write row to a particular 2-component value, and advances the
 * write row.
 *
 * It is an error for the write row to advance past the end of data.
 */
INLINE void GeomVertexWriter::
set_data2i(const LVecBase2i &data) {
  nassertv(has_column());
  _packer->set_data2i(inc_pointer(), data);
}

/**
 * Sets the write row to a particular 3-component value, and advances the
 * write row.
 *
 * It is an error for the write row to advance past the end of data.
 */
INLINE void GeomVertexWriter::
set_data3i(int a, int b, int c) {
  set_data3i(LVecBase3i(a, b, c));
}

/**
 * Sets the write row to a particular 3-component value, and advances the
 * write row.
 *
 * It is an error for the write row to advance past the end of data.
 */
INLINE void GeomVertexWriter::
set_data3i(const LVecBase3i &data) {
  nassertv(has_column());
  _packer->set_data3i(inc_pointer(), data);
}

/**
 * Sets the write row to a particular 4-component value, and advances the
 * write row.
 *
 * It is an error for the write row to advance past the end of data.
 */
INLINE void GeomVertexWriter::
set_data4i(int a, int b, int c, int d) {
  set_data4i(LVecBase4i(a, b, c, d));
}

/**
 * Sets the write row to a particular 4-component value, and advances the
 * write row.
 *
 * It is an error for the write row to advance past the end of data.
 */
INLINE void GeomVertexWriter::
set_data4i(const LVecBase4i &data) {
  nassertv(has_column());
  _packer->set_data4i(inc_pointer(), data);
}

/**
 * Sets the write row to a particular 1-component value, and advances the
 * write row.
 *
 * If the write row advances past the end of data, implicitly adds a new row
 * to the data.
 */
INLINE void GeomVertexWriter::
add_data1f(float data) {
  nassertv(has_column());
  _packer->set_data1f(inc_add_pointer(), data);
}

/**
 * Sets the write row to a particular 2-component value, and advances the
 * write row.
 *
 * If the write row advances past the end of data, implicitly adds a new row
 * to the data.
 */
INLINE void GeomVertexWriter::
add_data2f(float x, float y) {
  add_data2f(LVecBase2f(x, y));
}

/**
 * Sets the write row to a particular 2-component value, and advances the
 * write row.
 *
 * If the write row advances past the end of data, implicitly adds a new row
 * to the data.
 */
INLINE void GeomVertexWriter::
add_data2f(const LVecBase2f &data) {
  nassertv(has_column());
  _packer->set_data2f(inc_add_pointer(), data);
}

/**
 * Sets the write row to a particular 3-component value, and advances the
 * write row.
 *
 * If the write row advances past the end of data, implicitly adds a new row
 * to the data.
 */
INLINE void GeomVertexWriter::
add_data3f(float x, float y, float z) {
  add_data3f(LVecBase3f(x, y, z));
}

/**
 * Sets the write row to a particular 3-component value, and advances the
 * write row.
 *
 * If the write row advances past the end of data, implicitly adds a new row
 * to the data.
 */
INLINE void GeomVertexWriter::
add_data3f(const LVecBase3f &data) {
  nassertv(has_column());
  _packer->set_data3f(inc_add_pointer(), data);
}

/**
 * Sets the write row to a particular 4-component value, and advances the
 * write row.
 *
 * If the write row advances past the end of data, implicitly adds a new row
 * to the data.
 */
INLINE void GeomVertexWriter::
add_data4f(float x, float y, float z, float w) {
  add_data4f(LVecBase4f(x, y, z, w));
}

/**
 * Sets the write row to a particular 4-component value, and advances the
 * write row.
 *
 * If the write row advances past the end of data, implicitly adds a new row
 * to the data.
 */
INLINE void GeomVertexWriter::
add_data4f(const LVecBase4f &data) {
  nassertv(has_column());
  _packer->set_data4f(inc_add_pointer(), data);
}

/**
 * Sets the write row to a 3-by-3 matrix, and advances the write row.  This is
 * a special method that can only be used on matrix columns.
 *
 * If the write row advances past the end of data, implicitly adds a new row
 * to the data.
 */
INLINE void GeomVertexWriter::
add_matrix3f(const LMatrix3f &mat) {
  nassertv(has_column() &&
           _packer->_column->get_contents() == C_matrix &&
           _packer->_column->get_num_elements() == 3);

  size_t col_stride = _packer->_column->get_element_stride();
  unsigned char *pointer = inc_add_pointer();

  _packer->set_data3f(pointer, mat.get_row(0));
  pointer += col_stride;
  _packer->set_data3f(pointer, mat.get_row(1));
  pointer += col_stride;
  _packer->set_data3f(pointer, mat.get_row(2));
}

/**
 * Sets the write row to a 4-by-4 matrix, and advances the write row.  This is
 * a special method that can only be used on matrix columns.
 *
 * If the write row advances past the end of data, implicitly adds a new row
 * to the data.
 */
INLINE void GeomVertexWriter::
add_matrix4f(const LMatrix4f &mat) {
  nassertv(has_column() &&
           _packer->_column->get_contents() == C_matrix &&
           _packer->_column->get_num_elements() == 4);

  size_t col_stride = _packer->_column->get_element_stride();
  unsigned char *pointer = inc_add_pointer();

  _packer->set_data4f(pointer, mat.get_row(0));
  pointer += col_stride;
  _packer->set_data4f(pointer, mat.get_row(1));
  pointer += col_stride;
  _packer->set_data4f(pointer, mat.get_row(2));
  pointer += col_stride;
  _packer->set_data4f(pointer, mat.get_row(3));
}

/**
 * Sets the write row to a particular 1-component value, and advances the
 * write row.
 *
 * If the write row advances past the end of data, implicitly adds a new row
 * to the data.
 */
INLINE void GeomVertexWriter::
add_data1d(double data) {
  nassertv(has_column());
  _packer->set_data1d(inc_add_pointer(), data);
}

/**
 * Sets the write row to a particular 2-component value, and advances the
 * write row.
 *
 * If the write row advances past the end of data, implicitly adds a new row
 * to the data.
 */
INLINE void GeomVertexWriter::
add_data2d(double x, double y) {
  add_data2d(LVecBase2d(x, y));
}

/**
 * Sets the write row to a particular 2-component value, and advances the
 * write row.
 *
 * If the write row advances past the end of data, implicitly adds a new row
 * to the data.
 */
INLINE void GeomVertexWriter::
add_data2d(const LVecBase2d &data) {
  nassertv(has_column());
  _packer->set_data2d(inc_add_pointer(), data);
}

/**
 * Sets the write row to a particular 3-component value, and advances the
 * write row.
 *
 * If the write row advances past the end of data, implicitly adds a new row
 * to the data.
 */
INLINE void GeomVertexWriter::
add_data3d(double x, double y, double z) {
  add_data3d(LVecBase3d(x, y, z));
}

/**
 * Sets the write row to a particular 3-component value, and advances the
 * write row.
 *
 * If the write row advances past the end of data, implicitly adds a new row
 * to the data.
 */
INLINE void GeomVertexWriter::
add_data3d(const LVecBase3d &data) {
  nassertv(has_column());
  _packer->set_data3d(inc_add_pointer(), data);
}

/**
 * Sets the write row to a particular 4-component value, and advances the
 * write row.
 *
 * If the write row advances past the end of data, implicitly adds a new row
 * to the data.
 */
INLINE void GeomVertexWriter::
add_data4d(double x, double y, double z, double w) {
  add_data4d(LVecBase4d(x, y, z, w));
}

/**
 * Sets the write row to a particular 4-component value, and advances the
 * write row.
 *
 * If the write row advances past the end of data, implicitly adds a new row
 * to the data.
 */
INLINE void GeomVertexWriter::
add_data4d(const LVecBase4d &data) {
  nassertv(has_column());
  _packer->set_data4d(inc_add_pointer(), data);
}

/**
 * Sets the write row to a 3-by-3 matrix, and advances the write row.  This is
 * a special method that can only be used on matrix columns.
 *
 * If the write row advances past the end of data, implicitly adds a new row
 * to the data.
 */
INLINE void GeomVertexWriter::
add_matrix3d(const LMatrix3d &mat) {
  nassertv(has_column() &&
           _packer->_column->get_contents() == C_matrix &&
           _packer->_column->get_num_elements() == 3);

  size_t col_stride = _packer->_column->get_element_stride();
  unsigned char *pointer = inc_add_pointer();

  _packer->set_data3d(pointer, mat.get_row(0));
  pointer += col_stride;
  _packer->set_data3d(pointer, mat.get_row(1));
  pointer += col_stride;
  _packer->set_data3d(pointer, mat.get_row(2));
}

/**
 * Sets the write row to a 4-by-4 matrix, and advances the write row.  This is
 * a special method that can only be used on matrix columns.
 *
 * If the write row advances past the end of data, implicitly adds a new row
 * to the data.
 */
INLINE void GeomVertexWriter::
add_matrix4d(const LMatrix4d &mat) {
  nassertv(has_column() &&
           _packer->_column->get_contents() == C_matrix &&
           _packer->_column->get_num_elements() == 4);

  size_t col_stride = _packer->_column->get_element_stride();
  unsigned char *pointer = inc_add_pointer();

  _packer->set_data4d(pointer, mat.get_row(0));
  pointer += col_stride;
  _packer->set_data4d(pointer, mat.get_row(1));
  pointer += col_stride;
  _packer->set_data4d(pointer, mat.get_row(2));
  pointer += col_stride;
  _packer->set_data4d(pointer, mat.get_row(3));
}

/**
 * Sets the write row to a particular 1-component value, and advances the
 * write row.
 *
 * If the write row advances past the end of data, implicitly adds a new row
 * to the data.
 */
INLINE void GeomVertexWriter::
add_data1(PN_stdfloat data) {
#ifndef STDFLOAT_DOUBLE
  add_data1f(data);
#else
  add_data1d(data);
#endif
}

/**
 * Sets the write row to a particular 2-component value, and advances the
 * write row.
 *
 * If the write row advances past the end of data, implicitly adds a new row
 * to the data.
 */
INLINE void GeomVertexWriter::
add_data2(PN_stdfloat x, PN_stdfloat y) {
#ifndef STDFLOAT_DOUBLE
  add_data2f(x, y);
#else
  add_data2d(x, y);
#endif
}

/**
 * Sets the write row to a particular 2-component value, and advances the
 * write row.
 *
 * If the write row advances past the end of data, implicitly adds a new row
 * to the data.
 */
INLINE void GeomVertexWriter::
add_data2(const LVecBase2 &data) {
#ifndef STDFLOAT_DOUBLE
  add_data2f(data);
#else
  add_data2d(data);
#endif
}

/**
 * Sets the write row to a particular 3-component value, and advances the
 * write row.
 *
 * If the write row advances past the end of data, implicitly adds a new row
 * to the data.
 */
INLINE void GeomVertexWriter::
add_data3(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z) {
#ifndef STDFLOAT_DOUBLE
  add_data3f(x, y, z);
#else
  add_data3d(x, y, z);
#endif
}

/**
 * Sets the write row to a particular 3-component value, and advances the
 * write row.
 *
 * If the write row advances past the end of data, implicitly adds a new row
 * to the data.
 */
INLINE void GeomVertexWriter::
add_data3(const LVecBase3 &data) {
#ifndef STDFLOAT_DOUBLE
  add_data3f(data);
#else
  add_data3d(data);
#endif
}

/**
 * Sets the write row to a particular 4-component value, and advances the
 * write row.
 *
 * If the write row advances past the end of data, implicitly adds a new row
 * to the data.
 */
INLINE void GeomVertexWriter::
add_data4(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z, PN_stdfloat w) {
#ifndef STDFLOAT_DOUBLE
  add_data4f(x, y, z, w);
#else
  add_data4d(x, y, z, w);
#endif
}

/**
 * Sets the write row to a particular 4-component value, and advances the
 * write row.
 *
 * If the write row advances past the end of data, implicitly adds a new row
 * to the data.
 */
INLINE void GeomVertexWriter::
add_data4(const LVecBase4 &data) {
#ifndef STDFLOAT_DOUBLE
  add_data4f(data);
#else
  add_data4d(data);
#endif
}

/**
 * Sets the write row to a 3-by-3 matrix, and advances the write row.  This is
 * a special method that can only be used on matrix columns.
 *
 * If the write row advances past the end of data, implicitly adds a new row
 * to the data.
 */
INLINE void GeomVertexWriter::
add_matrix3(const LMatrix3 &mat) {
#ifndef STDFLOAT_DOUBLE
  add_matrix3f(mat);
#else
  add_matrix3d(mat);
#endif
}

/**
 * Sets the write row to a 4-by-4 matrix, and advances the write row.  This is
 * a special method that can only be used on matrix columns.
 *
 * If the write row advances past the end of data, implicitly adds a new row
 * to the data.
 */
INLINE void GeomVertexWriter::
add_matrix4(const LMatrix4 &mat) {
#ifndef STDFLOAT_DOUBLE
  add_matrix4f(mat);
#else
  add_matrix4d(mat);
#endif
}

/**
 * Sets the write row to a particular 1-component value, and advances the
 * write row.
 *
 * If the write row advances past the end of data, implicitly adds a new row
 * to the data.
 */
INLINE void GeomVertexWriter::
add_data1i(int data) {
  nassertv(has_column());
  _packer->set_data1i(inc_add_pointer(), data);
}

/**
 * Sets the write row to a particular 2-component value, and advances the
 * write row.
 *
 * If the write row advances past the end of data, implicitly adds a new row
 * to the data.
 */
INLINE void GeomVertexWriter::
add_data2i(int a, int b) {
  add_data2i(LVecBase2i(a, b));
}

/**
 * Sets the write row to a particular 2-component value, and advances the
 * write row.
 *
 * If the write row advances past the end of data, implicitly adds a new row
 * to the data.
 */
INLINE void GeomVertexWriter::
add_data2i(const LVecBase2i &data) {
  nassertv(has_column());
  _packer->set_data2i(inc_add_pointer(), data);
}

/**
 * Sets the write row to a particular 3-component value, and advances the
 * write row.
 *
 * If the write row advances past the end of data, implicitly adds a new row
 * to the data.
 */
INLINE void GeomVertexWriter::
add_data3i(int a, int b, int c) {
  add_data3i(LVecBase3i(a, b, c));
}

/**
 * Sets the write row to a particular 3-component value, and advances the
 * write row.
 *
 * If the write row advances past the end of data, implicitly adds a new row
 * to the data.
 */
INLINE void GeomVertexWriter::
add_data3i(const LVecBase3i &data) {
  nassertv(has_column());
  _packer->set_data3i(inc_add_pointer(), data);
}

/**
 * Sets the write row to a particular 4-component value, and advances the
 * write row.
 *
 * If the write row advances past the end of data, implicitly adds a new row
 * to the data.
 */
INLINE void GeomVertexWriter::
add_data4i(int a, int b, int c, int d) {
  add_data4i(LVecBase4i(a, b, c, d));
}

/**
 * Sets the write row to a particular 4-component value, and advances the
 * write row.
 *
 * If the write row advances past the end of data, implicitly adds a new row
 * to the data.
 */
INLINE void GeomVertexWriter::
add_data4i(const LVecBase4i &data) {
  nassertv(has_column());
  _packer->set_data4i(inc_add_pointer(), data);
}

/**
 * Returns the writer's Packer object.
 */
INLINE GeomVertexColumn::Packer *GeomVertexWriter::
get_packer() const {
  return _packer;
}

/**
 * Sets up the array pointers freshly from the source object (in case they
 * have been reallocated recently), and sets the internal pointer to the
 * indicated row.
 */
INLINE void GeomVertexWriter::
set_pointer(int row) {
  _pointer_begin = _handle->get_write_pointer();
  _pointer_end = _pointer_begin + _handle->get_data_size_bytes();
  quick_set_pointer(row);
}

/**
 * Sets up the internal pointer to the indicated row, without first verifying
 * that arrays haven't been reallocated.
 */
INLINE void GeomVertexWriter::
quick_set_pointer(int row) {
  nassertv(has_column());

#if defined(_DEBUG)
  // Make sure we still have the same pointer as stored in the array.
  nassertv(_pointer_begin == _handle->get_write_pointer());
#endif

  _pointer = _pointer_begin + _packer->_column->get_start() + _stride * row;

#if defined(_DEBUG)
  // We have to allow the pointer to exceed the end by up to one row's width,
  // because the next call might be to add_data_*().
  nassertv(_pointer_begin == _pointer_end || (_pointer - _packer->_column->get_start())  <= _pointer_end);
#endif
}

/**
 * Increments to the next row, and returns the data pointer as it was before
 * incrementing.
 */
INLINE unsigned char *GeomVertexWriter::
inc_pointer() {
#if defined(_DEBUG)
  nassertr(_pointer < _pointer_end, empty_buffer);
  // Make sure we still have the same pointer as stored in the array.
  nassertr(_pointer_begin == _handle->get_write_pointer(), empty_buffer);
  nassertr(_pointer < _pointer_begin + _handle->get_data_size_bytes(), empty_buffer);
#endif

  unsigned char *orig_pointer = _pointer;
  _pointer += _stride;
  return orig_pointer;
}

/**
 * Increments to the next row, and returns the data pointer as it was before
 * incrementing.  If we are at or past the end of data, implicitly adds more
 * rows first.
 */
INLINE unsigned char *GeomVertexWriter::
inc_add_pointer() {
  if (_pointer >= _pointer_end) {
    // Reset the data pointer.
    int write_row = get_write_row();

    if (_vertex_data != nullptr) {
      // If we have a whole GeomVertexData, we must set the length of all its
      // arrays at once.
      _handle = nullptr;
      GeomVertexDataPipelineWriter writer(_vertex_data, true, _current_thread);
      writer.check_array_writers();
      writer.set_num_rows(std::max(write_row + 1, writer.get_num_rows()));
      _handle = writer.get_array_writer(_array);

    } else {
      // Otherwise, we can get away with modifying only the one array we're
      // using.
      _handle->set_num_rows(std::max(write_row + 1, _handle->get_num_rows()));
    }

    set_pointer(write_row);
  }
  return inc_pointer();
}
