// Filename: drawMaskAttrib.I
// Created by:  drose (28Sep05)
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
//     Function: DrawMaskAttrib::Constructor
//       Access: Protected
//  Description: Use DrawMaskAttrib::make() to construct a new
//               DrawMaskAttrib object.
////////////////////////////////////////////////////////////////////
INLINE DrawMaskAttrib::
DrawMaskAttrib(DrawMask new_mask, DrawMask bits_to_change) :
  _new_mask(new_mask & bits_to_change),
  _bits_to_change(bits_to_change)
{
}

////////////////////////////////////////////////////////////////////
//     Function: DrawMaskAttrib::Copy Constructor
//       Access: Protected
//  Description: Use DrawMaskAttrib::make() to construct a new
//               DrawMaskAttrib object.
////////////////////////////////////////////////////////////////////
INLINE DrawMaskAttrib::
DrawMaskAttrib(const DrawMaskAttrib &copy) :
  _new_mask(copy._new_mask),
  _bits_to_change(copy._bits_to_change)
{
}

////////////////////////////////////////////////////////////////////
//     Function: DrawMaskAttrib::make_hide
//       Access: Published, Static
//  Description: Constructs a new DrawMaskAttrib that removes the
//               indicated draw bits from the visibility mask.  That
//               is, it makes any nodes invisible to cameras that have
//               any bits in common with draw_mask.  This is similar
//               to (but not quite identical to)
//               NodePath.hide(draw_mask).
////////////////////////////////////////////////////////////////////
INLINE CPT(RenderAttrib) DrawMaskAttrib::
make_hide(DrawMask draw_mask) {
  return make(DrawMask::all_off(), draw_mask);
}

////////////////////////////////////////////////////////////////////
//     Function: DrawMaskAttrib::make_show
//       Access: Published, Static
//  Description: Constructs a new DrawMaskAttrib that adds the
//               indicated draw bits to the visibility mask.  That
//               is, it makes any nodes visible to cameras that have
//               any bits in common with draw_mask.  This is similar
//               to (but not quite identical to)
//               NodePath.show(draw_mask).
////////////////////////////////////////////////////////////////////
INLINE CPT(RenderAttrib) DrawMaskAttrib::
make_show(DrawMask draw_mask) {
  return make(draw_mask, draw_mask);
}

////////////////////////////////////////////////////////////////////
//     Function: DrawMaskAttrib::get_new_mask
//       Access: Published
//  Description: Returns the new DrawMask that will be set after the
//               attrib has been applied.
////////////////////////////////////////////////////////////////////
INLINE DrawMask DrawMaskAttrib::
get_new_mask() const {
  return _new_mask;
}

////////////////////////////////////////////////////////////////////
//     Function: DrawMaskAttrib::get_bits_to_change
//       Access: Published
//  Description: Returns the set of bits that will be allowed to be
//               changed by this DrawMaskAttrib.
////////////////////////////////////////////////////////////////////
INLINE DrawMask DrawMaskAttrib::
get_bits_to_change() const {
  return _bits_to_change;
}
