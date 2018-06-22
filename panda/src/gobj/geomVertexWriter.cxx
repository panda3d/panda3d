/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomVertexWriter.cxx
 * @author drose
 * @date 2005-03-25
 */

#include "geomVertexWriter.h"

#ifdef _DEBUG
  // This is defined just for the benefit of having something non-NULL to
  // return from a nassertr() call.
unsigned char GeomVertexWriter::empty_buffer[100] = { 0 };
#endif

/**
 * Sets up the writer to use the indicated column description on the given
 * array.
 *
 * This also resets the current write row number to the start row (the same
 * value passed to a previous call to set_row(), or 0 if set_row() was never
 * called.)
 *
 * The return value is true if the data type is valid, false otherwise.
 */
bool GeomVertexWriter::
set_column(int array, const GeomVertexColumn *column) {
  if (_vertex_data == nullptr &&
      _array_data == nullptr) {
    return false;
  }

  if (column == nullptr) {
    // Clear the data type.
    _array = -1;
    _packer = nullptr;
    _stride = 0;
    _pointer = nullptr;
    _pointer_end = nullptr;

    return false;
  }

  if (_vertex_data != nullptr) {
    GeomVertexDataPipelineWriter writer(_vertex_data, true, _current_thread);
    writer.check_array_writers();
    return set_vertex_column(array, column, &writer);
  }
  if (_array_data != nullptr) {
    return set_array_column(column);
  }

  // No data is associated with the Writer.
  return false;
}

/**
 * This ensures that enough memory space for num_rows is allocated, so that
 * you may add up to num_rows rows without causing a new memory allocation.
 * This is a performance optimization only; it is especially useful when you
 * know the number of rows you will be adding ahead of time.
 */
bool GeomVertexWriter::
reserve_num_rows(int num_rows) {
  bool result;

  if (_vertex_data != nullptr) {
    // If we have a whole GeomVertexData, we must set the length of all its
    // arrays at once.
    GeomVertexDataPipelineWriter writer(_vertex_data, true, _current_thread);
    writer.check_array_writers();
    result = writer.reserve_num_rows(num_rows);
    _handle = writer.get_array_writer(_array);

  } else {
    // Otherwise, we can get away with modifying only the one array we're
    // using.
    result = _handle->reserve_num_rows(num_rows);
  }

  return result;
}

/**
 *
 */
void GeomVertexWriter::
output(std::ostream &out) const {
  const GeomVertexColumn *column = get_column();
  if (column == nullptr) {
    out << "GeomVertexWriter()";

  } else {
    out << "GeomVertexWriter, array = " << get_array_data()
        << ", column = " << column->get_name()
        << " (" << get_packer()->get_name()
        << "), write row " << get_write_row();
  }
}

/**
 * Called only by the constructor.
 */
void GeomVertexWriter::
initialize() {
  _array = 0;
  _packer = nullptr;
  _pointer_begin = nullptr;
  _pointer_end = nullptr;
  _pointer = nullptr;
  _start_row = 0;
}

/**
 * Internal method to set the column to column from the indicated array,
 * assuming we have a GeomVertexData
 */
bool GeomVertexWriter::
set_vertex_column(int array, const GeomVertexColumn *column,
                  GeomVertexDataPipelineWriter *data_writer) {
  if (column == nullptr) {
    return set_column(0, nullptr);
  }

  nassertr(_vertex_data != nullptr, false);

#ifndef NDEBUG
  _array = -1;
  _packer = nullptr;
  nassertr(array >= 0 && (size_t)array < _vertex_data->get_num_arrays(), false);
#endif

  _array = array;
  _handle = data_writer->get_array_writer(_array);
  _stride = _handle->get_array_format()->get_stride();

  _packer = column->_packer;
  set_pointer(_start_row);

  return true;
}

/**
 * Internal method to set the column to column from the indicated array,
 * assuming we have a GeomVertexArrayData.
 */
bool GeomVertexWriter::
set_array_column(const GeomVertexColumn *column) {
  if (column == nullptr) {
    return set_column(0, nullptr);
  }

  nassertr(_array_data != nullptr, false);

  _handle = _array_data->modify_handle();
  _stride = _handle->get_array_format()->get_stride();

  _packer = column->_packer;
  set_pointer(_start_row);

  return true;
}
