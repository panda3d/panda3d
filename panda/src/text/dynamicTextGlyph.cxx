// Filename: dynamicTextGlyph.cxx
// Created by:  drose (09Feb02)
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

#include "dynamicTextGlyph.h"

#ifdef HAVE_FREETYPE

#include "dynamicTextPage.h"
#include "geomTristrip.h"
#include "textureTransition.h"
#include "transparencyTransition.h"

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextGlyph::get_row
//       Access: Publiic
//  Description: Returns a pointer to the first byte in the pixel
//               buffer associated with the leftmost pixel in the
//               indicated row, where 0 is the topmost row and _y_size
//               - _margin * 2 - 1 is the bottommost row.
////////////////////////////////////////////////////////////////////
unsigned char *DynamicTextGlyph::
get_row(int y) {
  nassertr(y >= 0 && y < _y_size - _margin * 2, (unsigned char *)NULL);
  nassertr(_page != (DynamicTextPage *)NULL, (unsigned char *)NULL);
  nassertr(_page->_pbuffer != (PixelBuffer *)NULL, (unsigned char *)NULL);

  // First, offset y by the glyph's start.
  y += _y + _margin;
  // Also, get the x start.
  int x = _x + _margin;

  // Invert y.
  y = _page->_pbuffer->get_ysize() - 1 - y;

  int offset = (y * _page->_pbuffer->get_xsize()) + x;
  return _page->_pbuffer->_image + offset; 
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextGlyph::make_geom
//       Access: Publiic
//  Description: Creates the actual geometry for the glyph.  The
//               parameters bitmap_top and bitmap_left are from
//               FreeType, and indicate the position of the top left
//               corner of the bitmap relative to the glyph's origin.
//               The advance number represents the number of pixels
//               the pen should be advanced after drawing this glyph.
////////////////////////////////////////////////////////////////////
void DynamicTextGlyph::
make_geom(int bitmap_top, int bitmap_left, 
          float advance, float poly_margin, float pixels_per_unit) {
  // Determine the corners of the rectangle in geometric units.
  float top = (bitmap_top + poly_margin) / pixels_per_unit;
  float left = (bitmap_left - poly_margin) / pixels_per_unit;
  float bottom = (bitmap_top - _y_size - poly_margin) / pixels_per_unit;
  float right = (bitmap_left + _x_size + poly_margin) / pixels_per_unit;

  // And the corresponding corners in UV units.
  float uv_top = 1.0f - (float)(_y - poly_margin) / _page->get_y_size();
  float uv_left = (float)(_x - poly_margin) / _page->get_x_size();
  float uv_bottom = 1.0f - (float)(_y + _y_size + poly_margin) / _page->get_y_size();
  float uv_right = (float)(_x + _x_size + poly_margin) / _page->get_x_size();

  // Create a corresponding tristrip.
  _geom = new GeomTristrip;

  PTA_Vertexf coords;
  coords.push_back(Vertexf(left, 0, top));
  coords.push_back(Vertexf(left, 0, bottom));
  coords.push_back(Vertexf(right, 0, top));
  coords.push_back(Vertexf(right, 0, bottom));

  PTA_TexCoordf texcoords;
  texcoords.push_back(TexCoordf(uv_left, uv_top));
  texcoords.push_back(TexCoordf(uv_left, uv_bottom));
  texcoords.push_back(TexCoordf(uv_right, uv_top));
  texcoords.push_back(TexCoordf(uv_right, uv_bottom));

  PTA_Colorf colors;
  colors.push_back(Colorf(1.0f, 1.0f, 1.0f, 1.0f));

  PTA_int lengths;
  lengths.push_back(4);

  _geom->set_coords(coords);
  _geom->set_texcoords(texcoords, G_PER_VERTEX);
  _geom->set_colors(colors, G_OVERALL);
  _geom->set_lengths(lengths);
  _geom->set_num_prims(1);

  TextureTransition *tex = new TextureTransition(_page);
  TransparencyTransition *trans = new TransparencyTransition(TransparencyProperty::M_alpha);

  _trans.set_transition(tex);
  _trans.set_transition(trans);

  _advance = advance / pixels_per_unit;
}


#endif  // HAVE_FREETYPE
