// Filename: cardMaker.I
// Created by:  drose (16Mar02)
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
//     Function: CardMaker::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CardMaker::
CardMaker(const string &name) : Namable(name) {
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

////////////////////////////////////////////////////////////////////
//     Function: CardMaker::set_color
//       Access: Public
//  Description: Sets the color of the card.
////////////////////////////////////////////////////////////////////
INLINE void CardMaker::
set_color(float r, float g, float b, float a) {
  set_color(LVecBase4f(r, g, b, a));
} 

////////////////////////////////////////////////////////////////////
//     Function: CardMaker::set_color
//       Access: Public
//  Description: Sets the color of the card.
////////////////////////////////////////////////////////////////////
INLINE void CardMaker::
set_color(const LVecBase4f &color) {
  _color = color;
  _has_color = true;
}

////////////////////////////////////////////////////////////////////
//     Function: CardMaker::set_source_geometry
//       Access: Published
//  Description: Sets a node that will be copied (and scaled and
//               translated) to generate the frame, instead of
//               generating a new polygon.  The node may contain
//               arbitrary geometry that describes a flat polygon
//               contained within the indicated left, right, bottom,
//               top frame.
//
//               When generate() is called, the geometry in this node
//               will be scaled and translated appropriately to give
//               it the size and aspect ratio specified by
//               set_frame().
////////////////////////////////////////////////////////////////////
INLINE void CardMaker::
set_source_geometry(PandaNode *node, const LVecBase4f &frame) {
  _source_geometry = node;
  _source_frame = frame;
}

////////////////////////////////////////////////////////////////////
//     Function: CardMaker::clear_source_geometry
//       Access: Published
//  Description: Removes the node specified by an earlier call to
//               set_source_geometry().
////////////////////////////////////////////////////////////////////
INLINE void CardMaker::
clear_source_geometry() {
  _source_geometry = (PandaNode *)NULL;
}
