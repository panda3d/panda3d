// Filename: vertexTransform.I
// Created by:  drose (23Mar05)
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
//     Function: VertexTransform::get_modified
//       Access: Published
//  Description: Returns a sequence number that's guaranteed to change
//               at least every time the value reported by
//               get_matrix() changes.
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq VertexTransform::
get_modified() const {
  CDReader cdata(_cycler);
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
get_global_modified() {
  CDReader cdata(_global_cycler);
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
