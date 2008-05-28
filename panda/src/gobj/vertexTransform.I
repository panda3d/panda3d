// Filename: vertexTransform.I
// Created by:  drose (23Mar05)
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
//     Function: VertexTransform::get_modified
//       Access: Published
//  Description: Returns a sequence number that's guaranteed to change
//               at least every time the value reported by
//               get_matrix() changes.
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq VertexTransform::
get_modified(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return cdata->_modified;
}

////////////////////////////////////////////////////////////////////
//     Function: VertexTransform::get_global_modified
//       Access: Published, Static
//  Description: Returns the currently highest
//               VertexTransform::get_modified() value in the world.
//               This can be used as a quick way to determine if any
//               VertexTransforms have changed value recently.
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq VertexTransform::
get_global_modified(Thread *current_thread) {
  CDReader cdata(_global_cycler, current_thread);
  return cdata->_modified;
}

////////////////////////////////////////////////////////////////////
//     Function: VertexTransform::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE VertexTransform::CData::
CData() {
}

////////////////////////////////////////////////////////////////////
//     Function: VertexTransform::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE VertexTransform::CData::
CData(const VertexTransform::CData &copy) :
  _modified(copy._modified)
{
}

INLINE ostream &
operator << (ostream &out, const VertexTransform &obj) {
  obj.output(out);
  return out;
}
