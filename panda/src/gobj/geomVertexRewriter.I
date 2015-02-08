// Filename: geomVertexRewriter.I
// Created by:  drose (28Mar05)
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
//     Function: GeomVertexRewriter::Default Constructor
//       Access: Published
//  Description: Constructs an invalid GeomVertexRewriter.  You must use
//               the assignment operator to assign a valid
//               GeomVertexRewriter to this object before you can use
//               it.
////////////////////////////////////////////////////////////////////
INLINE GeomVertexRewriter::
GeomVertexRewriter(Thread *current_thread) :
  GeomVertexWriter(current_thread),
  GeomVertexReader(current_thread)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexRewriter::Constructor
//       Access: Published
//  Description: Constructs a new rewriter to process the vertices of
//               the indicated data object.
////////////////////////////////////////////////////////////////////
INLINE GeomVertexRewriter::
GeomVertexRewriter(GeomVertexData *vertex_data, Thread *current_thread) :
  GeomVertexWriter(vertex_data, current_thread),
  GeomVertexReader(vertex_data, current_thread)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexRewriter::Constructor
//       Access: Published
//  Description: Constructs a new rewriter to process the vertices of
//               the indicated data object.  This flavor creates the
//               rewriter specifically to process the named data type.
////////////////////////////////////////////////////////////////////
INLINE GeomVertexRewriter::
GeomVertexRewriter(GeomVertexData *vertex_data, CPT_InternalName name,
                   Thread *current_thread) :
  GeomVertexWriter(vertex_data, current_thread),
  GeomVertexReader(vertex_data, current_thread)
{
  set_column(MOVE(name));
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexRewriter::Constructor
//       Access: Published
//  Description: Constructs a new rewriter to process the vertices of
//               the indicated array only.
////////////////////////////////////////////////////////////////////
INLINE GeomVertexRewriter::
GeomVertexRewriter(GeomVertexArrayData *array_data, Thread *current_thread) :
  GeomVertexWriter(array_data, current_thread),
  GeomVertexReader(array_data, current_thread)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexRewriter::Constructor
//       Access: Published
//  Description: Constructs a new rewriter to process the vertices of
//               the indicated array only.
////////////////////////////////////////////////////////////////////
INLINE GeomVertexRewriter::
GeomVertexRewriter(GeomVertexArrayData *array_data, int column,
                   Thread *current_thread) :
  GeomVertexWriter(array_data, current_thread),
  GeomVertexReader(array_data, current_thread)
{
  set_column(column);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexRewriter::Copy Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE GeomVertexRewriter::
GeomVertexRewriter(const GeomVertexRewriter &copy) :
  GeomVertexWriter(copy),
  GeomVertexReader(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexRewriter::Copy Assignment Operator
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexRewriter::
operator = (const GeomVertexRewriter &copy) {
  GeomVertexWriter::operator = (copy);
  GeomVertexReader::operator = (copy);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexRewriter::Destructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE GeomVertexRewriter::
~GeomVertexRewriter() {
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexRewriter::get_vertex_data
//       Access: Published
//  Description: Returns the vertex data object that the
//               rewriter is processing.
////////////////////////////////////////////////////////////////////
INLINE GeomVertexData *GeomVertexRewriter::
get_vertex_data() const {
  nassertr(GeomVertexWriter::get_vertex_data() == 
           GeomVertexReader::get_vertex_data(), NULL);
  return GeomVertexWriter::get_vertex_data();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexRewriter::get_array_data
//       Access: Published
//  Description: Returns the particular array object that the
//               rewriter is currently processing.
////////////////////////////////////////////////////////////////////
INLINE GeomVertexArrayData *GeomVertexRewriter::
get_array_data() const {
  nassertr(GeomVertexWriter::get_array_data() == 
           GeomVertexReader::get_array_data(), NULL);
  return GeomVertexWriter::get_array_data();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexRewriter::get_array_handle
//       Access: Published
//  Description: Returns the write handle to the array object that the
//               rewriter is currently processing.  This low-level call
//               should be used with caution; be careful with
//               modifying the data in the handle out from under the
//               GeomVertexRewriter.
////////////////////////////////////////////////////////////////////
INLINE GeomVertexArrayDataHandle *GeomVertexRewriter::
get_array_handle() const {
  return GeomVertexWriter::get_array_handle();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexRewriter::get_stride
//       Access: Published
//  Description: Returns the per-row stride (bytes between consecutive
//               rows) of the underlying vertex array.  This low-level
//               information is normally not needed to use the
//               GeomVertexRewriter directly.
////////////////////////////////////////////////////////////////////
INLINE size_t GeomVertexRewriter::
get_stride() const {
  nassertr(GeomVertexWriter::get_stride() == GeomVertexReader::get_stride(), 0);
  return GeomVertexWriter::get_stride();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexRewriter::get_current_thread
//       Access: Published
//  Description: Returns the Thread pointer of the currently-executing
//               thread, as passed to the constructor of this object.
////////////////////////////////////////////////////////////////////
INLINE Thread *GeomVertexRewriter::
get_current_thread() const {
  nassertr(GeomVertexWriter::get_current_thread() == 
           GeomVertexReader::get_current_thread(), NULL);
  return GeomVertexWriter::get_current_thread();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexRewriter::set_column
//       Access: Published
//  Description: Sets up the rewriter to use the nth data type of the
//               GeomVertexFormat, numbering from 0.
//
//               This also resets both the read and write row
//               numbers to the start row (the same value passed to
//               a previous call to set_row(), or 0 if set_row()
//               was never called.)
//
//               The return value is true if the data type is valid,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool GeomVertexRewriter::
set_column(int column) {
  // It's important to invoke the writer first, then the reader.  See
  // set_row().
  GeomVertexWriter::set_column(column);
  return GeomVertexReader::set_column(column);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexRewriter::set_column
//       Access: Published
//  Description: Sets up the rewriter to use the data type with the
//               indicated name.
//
//               This also resets both the read and write row
//               numbers to the start row (the same value passed to
//               a previous call to set_row(), or 0 if set_row()
//               was never called.)
//
//               The return value is true if the data type is valid,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool GeomVertexRewriter::
set_column(CPT_InternalName name) {
  // It's important to invoke the writer first, then the reader.  See
  // set_row().
  GeomVertexWriter::set_column(name);
  return GeomVertexReader::set_column(MOVE(name));
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexRewriter::set_column
//       Access: Published
//  Description: Sets up the rewriter to use the indicated column
//               description on the given array.
//
//               This also resets both the read and write row
//               numbers to the start row (the same value passed to
//               a previous call to set_row(), or 0 if set_row()
//               was never called.)
//
//               The return value is true if the data type is valid,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool GeomVertexRewriter::
set_column(int array, const GeomVertexColumn *column) {
  // It's important to invoke the writer first, then the reader.  See
  // set_row().
  GeomVertexWriter::set_column(array, column);
  return GeomVertexReader::set_column(array, column);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexRewriter::clear
//       Access: Published
//  Description: Resets the GeomVertexRewriter to the initial state.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexRewriter::
clear() {
  GeomVertexWriter::clear();
  GeomVertexReader::clear();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexRewriter::has_column
//       Access: Published
//  Description: Returns true if a valid data type has been
//               successfully set, or false if the data type does not
//               exist.
////////////////////////////////////////////////////////////////////
INLINE bool GeomVertexRewriter::
has_column() const {
  nassertr(GeomVertexWriter::get_column() == 
           GeomVertexReader::get_column(), false);
  return GeomVertexWriter::has_column();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexRewriter::get_array
//       Access: Published
//  Description: Returns the array index containing the data type that
//               the rewriter is working on.
////////////////////////////////////////////////////////////////////
INLINE int GeomVertexRewriter::
get_array() const {
  nassertr(GeomVertexWriter::get_array() == 
           GeomVertexReader::get_array(), -1);
  return GeomVertexWriter::get_array();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexRewriter::get_column
//       Access: Published
//  Description: Returns the description of the data type that the
//               rewriter is working on.
////////////////////////////////////////////////////////////////////
INLINE const GeomVertexColumn *GeomVertexRewriter::
get_column() const {
  nassertr(GeomVertexWriter::get_column() == 
           GeomVertexReader::get_column(), NULL);
  return GeomVertexWriter::get_column();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexRewriter::set_row_unsafe
//       Access: Published
//  Description: Sets the start row to the indicated value, without
//               internal checks.  This is the same as set_row(), but
//               it does not check for the possibility that the array
//               has been reallocated internally for some reason; use
//               only when you are confident that the array is
//               unchanged and you really need every bit of available
//               performance.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexRewriter::
set_row_unsafe(int row) {
  // It's important to invoke the Writer first, since that might force
  // a recopy of the array, which might invalidate the pointer already
  // stored by the Reader if we invoked the Reader first.
  GeomVertexWriter::set_row_unsafe(row);
  GeomVertexReader::set_row_unsafe(row);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexRewriter::set_row
//       Access: Published
//  Description: Sets the start, write, and write index to the
//               indicated value.  The rewriter will begin traversing
//               from the given row.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexRewriter::
set_row(int row) {
  GeomVertexWriter::set_row(row);
  GeomVertexReader::set_row(row);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexRewriter::get_start_row
//       Access: Published
//  Description: Returns the row index at which the rewriter
//               started.  It will return to this row if you reset
//               the current column.
////////////////////////////////////////////////////////////////////
INLINE int GeomVertexRewriter::
get_start_row() const {
  nassertr(GeomVertexWriter::get_start_row() == 
           GeomVertexReader::get_start_row(), 0);
  return GeomVertexWriter::get_start_row();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexRewriter::is_at_end
//       Access: Published
//  Description: Returns true if the reader or writer is currently at
//               the end of the list of vertices, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool GeomVertexRewriter::
is_at_end() const {
  return GeomVertexWriter::is_at_end() || GeomVertexReader::is_at_end();
}
