// Filename: cardMaker.I
// Created by:  drose (06Jul01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: CardMaker::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CardMaker::
CardMaker() {
  reset();
}

////////////////////////////////////////////////////////////////////
//     Function: CardMaker::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CardMaker::
~CardMaker() {
}

////////////////////////////////////////////////////////////////////
//     Function: CardMaker::set_uv_range
//       Access: Public
//  Description: Sets the range of UV's that will be applied to the
//               vertices.  If set_has_uvs() is true (as it is by
//               default), the vertices will be generated with the
//               indicated range of UV's, which will be useful if a
//               texture is applied.
////////////////////////////////////////////////////////////////////
INLINE void CardMaker::
set_uv_range(const TexCoordf &ll, const TexCoordf &ur) {
  _ll = ll;
  _ur = ur;
}

////////////////////////////////////////////////////////////////////
//     Function: CardMaker::set_has_uvs
//       Access: Public
//  Description: Sets the flag indicating whether vertices will be
//               generated with UV's or not.
////////////////////////////////////////////////////////////////////
INLINE void CardMaker::
set_has_uvs(bool flag) {
  _has_uvs = flag;
}

////////////////////////////////////////////////////////////////////
//     Function: CardMaker::set_frame
//       Access: Public
//  Description: Sets the size of the card.
////////////////////////////////////////////////////////////////////
INLINE void CardMaker::
set_frame(float left, float right, float bottom, float top) {
  set_frame(LVecBase4f(left, right, bottom, top));
} 

////////////////////////////////////////////////////////////////////
//     Function: CardMaker::set_frame
//       Access: Public
//  Description: Sets the size of the card.
////////////////////////////////////////////////////////////////////
INLINE void CardMaker::
set_frame(const LVecBase4f &frame) {
  _frame = frame;
}
