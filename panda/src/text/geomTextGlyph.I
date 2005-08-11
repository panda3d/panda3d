// Filename: geomTextGlyph.I
// Created by:  drose (31Mar05)
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
//     Function: GeomTextGlyph::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE GeomTextGlyph::
GeomTextGlyph(DynamicTextGlyph *glyph, const GeomVertexData *data) :
  Geom(data),
  _glyph(glyph)
{
  if (_glyph != (DynamicTextGlyph *)NULL) {
    _glyph->_geom_count++;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTextGlyph::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE GeomTextGlyph::
GeomTextGlyph(const GeomTextGlyph &copy) :
  Geom(copy),
  _glyph(copy._glyph)
{
  if (_glyph != (DynamicTextGlyph *)NULL) {
    _glyph->_geom_count++;
  }
}
