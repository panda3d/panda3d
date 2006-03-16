// Filename: vertexBufferContext.I
// Created by:  drose (17Mar05)
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
//     Function: VertexBufferContext::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE VertexBufferContext::
VertexBufferContext(PreparedGraphicsObjects *pgo, GeomVertexArrayData *data) :
  BufferContext(&pgo->_vbuffer_residency),
  _data(data)
{
}

////////////////////////////////////////////////////////////////////
//     Function: VertexBufferContext::get_data
//       Access: Public
//  Description: Returns the pointer to the client-side array data
//               object.
////////////////////////////////////////////////////////////////////
INLINE GeomVertexArrayData *VertexBufferContext::
get_data() const {
  return _data;
}

////////////////////////////////////////////////////////////////////
//     Function: VertexBufferContext::changed_size
//       Access: Public
//  Description: Returns true if the data has changed size since the
//               last time mark_loaded() was called.
////////////////////////////////////////////////////////////////////
INLINE bool VertexBufferContext::
changed_size() const {
  return get_data_size_bytes() != (size_t)_data->get_data_size_bytes();
}

////////////////////////////////////////////////////////////////////
//     Function: VertexBufferContext::changed_usage_hint
//       Access: Public
//  Description: Returns true if the data has changed its usage hint
//               since the last time mark_loaded() was called.
////////////////////////////////////////////////////////////////////
INLINE bool VertexBufferContext::
changed_usage_hint() const {
  return _usage_hint != _data->get_usage_hint();
}

////////////////////////////////////////////////////////////////////
//     Function: VertexBufferContext::was_modified
//       Access: Public
//  Description: Returns true if the data has been modified since the
//               last time mark_loaded() was called.
////////////////////////////////////////////////////////////////////
INLINE bool VertexBufferContext::
was_modified() const {
  return get_modified() != _data->get_modified();
}

////////////////////////////////////////////////////////////////////
//     Function: VertexBufferContext::mark_loaded
//       Access: Public
//  Description: Should be called after the VertexBufferContext has been
//               loaded into graphics memory, this updates the
//               internal flags for changed_size() and modified().
////////////////////////////////////////////////////////////////////
INLINE void VertexBufferContext::
mark_loaded() {
  update_data_size_bytes(_data->get_data_size_bytes());
  update_modified(_data->get_modified());
  _usage_hint = _data->get_usage_hint();

  // Assume the buffer is now resident.
  set_resident(true);
}
