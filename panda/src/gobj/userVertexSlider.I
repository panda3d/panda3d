// Filename: userVertexSlider.I
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
//     Function: UserVertexSlider::set_slider
//       Access: Published
//  Description: Stores the indicated slider value.
////////////////////////////////////////////////////////////////////
INLINE void UserVertexSlider::
set_slider(float slider) {
  CDWriter cdata(_cycler);
  cdata->_slider = slider;
  mark_modified();
}

////////////////////////////////////////////////////////////////////
//     Function: UserVertexSlider::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE UserVertexSlider::CData::
CData() :
  _slider(0.0f)
{
}

////////////////////////////////////////////////////////////////////
//     Function: UserVertexSlider::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE UserVertexSlider::CData::
CData(const UserVertexSlider::CData &copy) :
  _slider(copy._slider)
{
}
