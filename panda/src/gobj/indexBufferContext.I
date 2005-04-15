// Filename: indexBufferContext.I
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
//     Function: IndexBufferContext::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE IndexBufferContext::
IndexBufferContext(qpGeomPrimitive *data) :
  _data(data),
  // Initially, the number of bytes is zero, until the data has been
  // loaded (and mark_loaded() is called).
  _data_size_bytes(0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: IndexBufferContext::get_data
//       Access: Public
//  Description: Returns the pointer to the client-side array data
//               object.
////////////////////////////////////////////////////////////////////
INLINE qpGeomPrimitive *IndexBufferContext::
get_data() const {
  return _data;
}

////////////////////////////////////////////////////////////////////
//     Function: IndexBufferContext::get_data_size_bytes
//       Access: Public
//  Description: Returns the number of bytes previously reported for
//               the data object.  This is used to track changes in
//               the data object's allocated size; if it changes from
//               this, we need to create a new buffer.
////////////////////////////////////////////////////////////////////
INLINE int IndexBufferContext::
get_data_size_bytes() const {
  return _data_size_bytes;
}

////////////////////////////////////////////////////////////////////
//     Function: IndexBufferContext::changed_size
//       Access: Public
//  Description: Returns true if the data has changed size since the
//               last time mark_loaded() was called.
////////////////////////////////////////////////////////////////////
INLINE bool IndexBufferContext::
changed_size() const {
  return get_data_size_bytes() != _data->get_data_size_bytes();
}

////////////////////////////////////////////////////////////////////
//     Function: IndexBufferContext::changed_usage_hint
//       Access: Public
//  Description: Returns true if the data has changed its usage hint
//               since the last time mark_loaded() was called.
////////////////////////////////////////////////////////////////////
INLINE bool IndexBufferContext::
changed_usage_hint() const {
  return _usage_hint != _data->get_usage_hint();
}

////////////////////////////////////////////////////////////////////
//     Function: IndexBufferContext::was_modified
//       Access: Public
//  Description: Returns true if the data has been modified since the
//               last time mark_loaded() was called.
////////////////////////////////////////////////////////////////////
INLINE bool IndexBufferContext::
was_modified() const {
  return _modified != _data->get_modified();
}

////////////////////////////////////////////////////////////////////
//     Function: IndexBufferContext::get_modified
//       Access: Public
//  Description: Returns the UpdateSeq that was recorded the last time
//               mark_loaded() was called.
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq IndexBufferContext::
get_modified() const {
  return _modified;
}

////////////////////////////////////////////////////////////////////
//     Function: IndexBufferContext::mark_loaded
//       Access: Public
//  Description: Should be called after the IndexBufferContext has been
//               loaded into graphics memory, this updates the
//               internal flags for changed_size() and modified().
////////////////////////////////////////////////////////////////////
INLINE void IndexBufferContext::
mark_loaded() {
  _data_size_bytes = _data->get_data_size_bytes();
  _usage_hint = _data->get_usage_hint();
  _modified = _data->get_modified();
}
