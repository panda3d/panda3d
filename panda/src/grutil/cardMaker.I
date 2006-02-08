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
//     Function: CardMaker::set_uv_range
//       Access: Public
//  Description: Sets the range of UV's that will be applied to the
//               vertices.  If set_has_uvs() is true (as it is by
//               default), the vertices will be generated with the
//               indicated range of UV's, which will be useful if a
//               texture is applied.
////////////////////////////////////////////////////////////////////
INLINE void CardMaker::
set_uv_range(const TexCoord3f &ll, const TexCoord3f &lr, const TexCoord3f &ur, const TexCoord3f &ul) {
  _ll_tex = ll;
  _lr_tex = lr;
  _ur_tex = ur;
  _ul_tex = ul;
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
set_uv_range(const TexCoordf &ll, const TexCoordf &lr, const TexCoordf &ur, const TexCoordf &ul) {
  _ll_tex.set(ll[0], ll[1], 0.0f);
  _lr_tex.set(lr[0], lr[1], 0.0f);
  _ur_tex.set(ur[0], ur[1], 0.0f);
  _ul_tex.set(ul[0], ul[1], 0.0f);
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
  _ll_tex.set(ll[0], ll[1], 0.0f);
  _lr_tex.set(ur[0], ll[1], 0.0f);
  _ur_tex.set(ur[0], ur[1], 0.0f);
  _ul_tex.set(ll[0], ur[1], 0.0f);
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
set_uv_range(const LVector4f &x, const LVector4f &y, const LVector4f &z) {
  _ll_tex.set(x[0], y[0], z[0]);
  _lr_tex.set(x[1], y[1], z[1]);
  _ur_tex.set(x[2], y[2], z[2]);
  _ul_tex.set(x[3], y[3], z[3]);
}

////////////////////////////////////////////////////////////////////
//     Function: CardMaker::set_uv_range_cube
//       Access: Public
//  Description: Sets the range of UV's that will be applied to the
//               vertices appropriately for a cube-map face.
////////////////////////////////////////////////////////////////////
INLINE void CardMaker::
set_uv_range_cube(int face) {
  LVector4f varya(-1,  1,  1, -1);
  LVector4f varyb(-1, -1,  1,  1);
  LVector4f fixed( 1,  1,  1,  1);
  switch(face) {
  case 0: set_uv_range( fixed, -varyb,  varya); break; // positive_x
  case 1: set_uv_range(-fixed, -varyb, -varya); break; // negative_x
  case 2: set_uv_range(-varya,  fixed,  varyb); break; // positive_y
  case 3: set_uv_range(-varya, -fixed, -varyb); break; // negative_y
  case 4: set_uv_range(-varya, -varyb,  fixed); break; // positive_z
  case 5: set_uv_range( varya, -varyb, -fixed); break; // negative_z
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CardMaker::set_frame
//       Access: Public
//  Description: Sets the size of the card.
////////////////////////////////////////////////////////////////////
INLINE void CardMaker::
set_frame(float left, float right, float bottom, float top) {
  _ll_pos.set(left,  0.0f, bottom);
  _lr_pos.set(right, 0.0f, bottom);
  _ur_pos.set(right, 0.0f, top);
  _ul_pos.set(left,  0.0f, top);
} 

////////////////////////////////////////////////////////////////////
//     Function: CardMaker::set_frame
//       Access: Public
//  Description: Sets the size of the card.
////////////////////////////////////////////////////////////////////
INLINE void CardMaker::
set_frame(const LVecBase4f &frame) {
  set_frame(frame[0], frame[1], frame[2], frame[3]);
}

////////////////////////////////////////////////////////////////////
//     Function: CardMaker::set_frame
//       Access: Public
//  Description: Sets the size of the card.
////////////////////////////////////////////////////////////////////
INLINE void CardMaker::
set_frame(const Vertexf &ll, const Vertexf &lr, const Vertexf &ur, const Vertexf &ul)
{
  _ll_pos = ll;
  _lr_pos = lr;
  _ur_pos = ur;
  _ul_pos = ul;
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
//     Function: CardMaker::set_has_normals
//       Access: Public
//  Description: Sets the flag indicating whether vertices will be
//               generated with normals or not.  Normals are required
//               if you intend to enable lighting on the card, but are
//               just wasted space and bandwidth otherwise, so there
//               is a (slight) optimization for disabling them.  If
//               enabled, the normals will be generated perpendicular
//               to the card's face.
////////////////////////////////////////////////////////////////////
INLINE void CardMaker::
set_has_normals(bool flag) {
  _has_normals = flag;
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
