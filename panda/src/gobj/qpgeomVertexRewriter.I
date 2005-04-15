// Filename: qpgeomVertexRewriter.I
// Created by:  drose (28Mar05)
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


////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexRewriter::Constructor
//       Access: Published
//  Description: Constructs a new rewriter to process the vertices of
//               the indicated data object.
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexRewriter::
qpGeomVertexRewriter(qpGeomVertexData *vertex_data) :
  qpGeomVertexWriter(vertex_data),
  qpGeomVertexReader(vertex_data)
{
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexRewriter::Constructor
//       Access: Published
//  Description: Constructs a new rewriter to process the vertices of
//               the indicated data object.  This flavor creates the
//               rewriter specifically to process the named data type.
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexRewriter::
qpGeomVertexRewriter(qpGeomVertexData *vertex_data, const string &name) :
  qpGeomVertexWriter(vertex_data),
  qpGeomVertexReader(vertex_data)
{
  set_column(name);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexRewriter::Constructor
//       Access: Published
//  Description: Constructs a new rewriter to process the vertices of
//               the indicated data object.  This flavor creates the
//               rewriter specifically to process the named data type.
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexRewriter::
qpGeomVertexRewriter(qpGeomVertexData *vertex_data, const InternalName *name) :
  qpGeomVertexWriter(vertex_data),
  qpGeomVertexReader(vertex_data)
{
  set_column(name);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexRewriter::Constructor
//       Access: Published
//  Description: Constructs a new rewriter to process the vertices of
//               the indicated array only.
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexRewriter::
qpGeomVertexRewriter(qpGeomVertexArrayData *array_data) :
  qpGeomVertexWriter(array_data),
  qpGeomVertexReader(array_data)
{
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexRewriter::Constructor
//       Access: Published
//  Description: Constructs a new rewriter to process the vertices of
//               the indicated array only.
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexRewriter::
qpGeomVertexRewriter(qpGeomVertexArrayData *array_data, int column) :
  qpGeomVertexWriter(array_data),
  qpGeomVertexReader(array_data)
{
  set_column(column);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexRewriter::Destructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexRewriter::
~qpGeomVertexRewriter() {
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexRewriter::get_vertex_data
//       Access: Published
//  Description: Returns the vertex data object that the
//               rewriter is processing.
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexData *qpGeomVertexRewriter::
get_vertex_data() const {
  nassertr(qpGeomVertexWriter::get_vertex_data() == 
           qpGeomVertexReader::get_vertex_data(), NULL);
  return qpGeomVertexWriter::get_vertex_data();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexRewriter::get_array_data
//       Access: Published
//  Description: Returns the particular array object that the
//               rewriter is currently processing.
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexArrayData *qpGeomVertexRewriter::
get_array_data() const {
  nassertr(qpGeomVertexWriter::get_array_data() == 
           qpGeomVertexReader::get_array_data(), NULL);
  return qpGeomVertexWriter::get_array_data();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexRewriter::set_column
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
INLINE bool qpGeomVertexRewriter::
set_column(int column) {
  // It's important to invoke the writer first, then the reader.  See
  // set_row().
  qpGeomVertexWriter::set_column(column);
  return qpGeomVertexReader::set_column(column);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexRewriter::set_column
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
INLINE bool qpGeomVertexRewriter::
set_column(const string &name) {
  return set_column(InternalName::make(name));
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexRewriter::set_column
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
INLINE bool qpGeomVertexRewriter::
set_column(const InternalName *name) {
  // It's important to invoke the writer first, then the reader.  See
  // set_row().
  qpGeomVertexWriter::set_column(name);
  return qpGeomVertexReader::set_column(name);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexRewriter::set_column
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
INLINE bool qpGeomVertexRewriter::
set_column(int array, const qpGeomVertexColumn *column) {
  // It's important to invoke the writer first, then the reader.  See
  // set_row().
  qpGeomVertexWriter::set_column(array, column);
  return qpGeomVertexReader::set_column(array, column);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexRewriter::has_column
//       Access: Published
//  Description: Returns true if a valid data type has been
//               successfully set, or false if the data type does not
//               exist.
////////////////////////////////////////////////////////////////////
INLINE bool qpGeomVertexRewriter::
has_column() const {
  nassertr(qpGeomVertexWriter::get_column() == 
           qpGeomVertexReader::get_column(), false);
  return qpGeomVertexWriter::has_column();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexRewriter::get_array
//       Access: Published
//  Description: Returns the array index containing the data type that
//               the rewriter is working on.
////////////////////////////////////////////////////////////////////
INLINE int qpGeomVertexRewriter::
get_array() const {
  nassertr(qpGeomVertexWriter::get_array() == 
           qpGeomVertexReader::get_array(), -1);
  return qpGeomVertexWriter::get_array();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexRewriter::get_column
//       Access: Published
//  Description: Returns the description of the data type that the
//               rewriter is working on.
////////////////////////////////////////////////////////////////////
INLINE const qpGeomVertexColumn *qpGeomVertexRewriter::
get_column() const {
  nassertr(qpGeomVertexWriter::get_column() == 
           qpGeomVertexReader::get_column(), NULL);
  return qpGeomVertexWriter::get_column();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexRewriter::set_row
//       Access: Published
//  Description: Sets the start, write, and write index to the
//               indicated value.  The rewriter will begin traversing
//               from the given row.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexRewriter::
set_row(int row) {
  // It's important to invoke the Writer first, since that might force
  // a recopy of the array, which might invalidate the pointer already
  // stored by the Reader if we invoked the Reader first.
  qpGeomVertexWriter::set_row(row);
  qpGeomVertexReader::set_row(row);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexRewriter::get_start_row
//       Access: Published
//  Description: Returns the row index at which the rewriter
//               started.  It will return to this row if you reset
//               the current column.
////////////////////////////////////////////////////////////////////
INLINE int qpGeomVertexRewriter::
get_start_row() const {
  nassertr(qpGeomVertexWriter::get_start_row() == 
           qpGeomVertexReader::get_start_row(), 0);
  return qpGeomVertexWriter::get_start_row();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexRewriter::is_at_end
//       Access: Published
//  Description: Returns true if the reader or writer is currently at
//               the end of the list of vertices, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool qpGeomVertexRewriter::
is_at_end() const {
  return qpGeomVertexWriter::is_at_end() || qpGeomVertexReader::is_at_end();
}
