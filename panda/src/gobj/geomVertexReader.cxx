/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomVertexReader.cxx
 * @author drose
 * @date 2005-03-25
 */

#include "geomVertexReader.h"

#ifdef _DEBUG
// This is defined just for the benefit of having something non-NULL to
// return from a nassertr() call.
const unsigned char GeomVertexReader::empty_buffer[100] = { 0 };
#endif

/**
 * Sets up the reader to use the indicated column description on the given
 * array.
 *
 * This also resets the current read row number to the start row (the same
 * value passed to a previous call to set_row(), or 0 if set_row() was never
 * called.)
 *
 * The return value is true if the data type is valid, false otherwise.
 */
bool GeomVertexReader::
set_column(int array, const GeomVertexColumn *column) {
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
    GeomVertexDataPipelineReader reader(_vertex_data, _current_thread);
    reader.check_array_readers();
    return set_vertex_column(array, column, &reader);
  }
  if (_array_data != nullptr) {
    return set_array_column(column);
  }

  // No data is associated with the Reader.
  return false;
}

/**
 *
 */
void GeomVertexReader::
output(std::ostream &out) const {
  const GeomVertexColumn *column = get_column();
  if (column == nullptr) {
    out << "GeomVertexReader()";

  } else {
    out << "GeomVertexReader, array = " << get_array_data()
        << ", column = " << column->get_name()
        << " (" << get_packer()->get_name()
        << "), read row " << get_read_row();
  }
}

/**
 * Called only by the constructor.
 */
void GeomVertexReader::
initialize() {
  _array = 0;
  _packer = nullptr;
  _pointer_begin = nullptr;
  _pointer_end = nullptr;
  _pointer = nullptr;
  _start_row = 0;
  _force = true;
}

/**
 * Internal method to set the column to column from the indicated array,
 * assuming we have a GeomVertexData
 */
bool GeomVertexReader::
set_vertex_column(int array, const GeomVertexColumn *column,
                  const GeomVertexDataPipelineReader *data_reader) {
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
  _handle = data_reader->get_array_reader(_array);
  _stride = _handle->get_array_format()->get_stride();

  _packer = column->_packer;
  return set_pointer(_start_row);
}

/**
 * Internal method to set the column to column from the indicated array,
 * assuming we have a GeomVertexArrayData.
 */
bool GeomVertexReader::
set_array_column(const GeomVertexColumn *column) {
  if (column == nullptr) {
    return set_column(0, nullptr);
  }

  nassertr(_array_data != nullptr, false);

  _handle = _array_data->get_handle();
  _stride = _handle->get_array_format()->get_stride();

  _packer = column->_packer;
  return set_pointer(_start_row);
}
