// Filename: dynamicTextGlyph.I
// Created by:  drose (09Feb02)
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
//     Function: DynamicTextGlyph::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE DynamicTextGlyph::
DynamicTextGlyph(int character, DynamicTextPage *page, int x, int y,
                 int x_size, int y_size, int margin) :
  TextGlyph(character),
  _page(page),
  _x(x), _y(y),
  _x_size(x_size), _y_size(y_size),
  _margin(margin)
{
  _geom_count = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextGlyph::Constructor
//       Access: Public
//  Description: This constructor makes an empty glyph, whose only
//               purpose is to remember its width.  It has no bitmap
//               and no Geom.
////////////////////////////////////////////////////////////////////
INLINE DynamicTextGlyph::
DynamicTextGlyph(int character, PN_stdfloat advance) :
  TextGlyph(character),
  _page((DynamicTextPage *)NULL),
  _x(0), _y(0),
  _x_size(0), _y_size(0),
  _margin(0)
{
  _advance = advance;
  _geom_count = 1;
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextGlyph::Copy Constructor
//       Access: Private
//  Description: Copying DynamicTextGlyph objects is not allowed.
////////////////////////////////////////////////////////////////////
INLINE DynamicTextGlyph::
DynamicTextGlyph(const DynamicTextGlyph &) :
  TextGlyph(0)
{
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextGlyph::Copy Assignment Operator
//       Access: Private
//  Description: Copying DynamicTextGlyph objects is not allowed.
////////////////////////////////////////////////////////////////////
INLINE void DynamicTextGlyph::
operator = (const DynamicTextGlyph &) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextGlyph::intersects
//       Access: Public
//  Description: Returns true if the particular position this glyph
//               has been assigned to overlaps the rectangle whose
//               top left corner is at x, y and whose size is given by
//               x_size, y_size, or false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool DynamicTextGlyph::
intersects(int x, int y, int x_size, int y_size) const {
  int hright = x + x_size;
  int hbot = y + y_size;

  int mright = _x + _x_size;
  int mbot = _y + _y_size;

  return !(x >= mright || hright <= _x ||
           y >= mbot || hbot <= _y);
}
