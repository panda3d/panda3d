// Filename: vertexSlider.I
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
//     Function: VertexSlider::get_name
//       Access: Published
//  Description: Returns the name of this particular slider.  Every
//               unique blend shape within a particular Geom must be
//               identified with a different name, which is shared by
//               the slider that controls it.
////////////////////////////////////////////////////////////////////
INLINE const InternalName *VertexSlider::
get_name() const {
  return _name;
}

////////////////////////////////////////////////////////////////////
//     Function: VertexSlider::get_modified
//       Access: Published
//  Description: Returns a sequence number that's guaranteed to change
//               at least every time the value reported by
//               get_slider() changes.
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq VertexSlider::
get_modified() const {
  CDReader cdata(_cycler);
  return cdata->_modified;
}

////////////////////////////////////////////////////////////////////
//     Function: VertexSlider::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE VertexSlider::CData::
CData() {
}

////////////////////////////////////////////////////////////////////
//     Function: VertexSlider::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE VertexSlider::CData::
CData(const VertexSlider::CData &copy) :
  _modified(copy._modified)
{
}

INLINE ostream &
operator << (ostream &out, const VertexSlider &obj) {
  obj.output(out);
  return out;
}
