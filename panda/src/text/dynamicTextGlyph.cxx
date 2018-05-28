/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dynamicTextGlyph.cxx
 * @author drose
 * @date 2002-02-09
 */

#include "dynamicTextGlyph.h"

#ifdef HAVE_FREETYPE

#include "dynamicTextFont.h"
#include "dynamicTextPage.h"
#include "geomTextGlyph.h"
#include "geomVertexData.h"
#include "geomVertexFormat.h"
#include "geomTriangles.h"
#include "geomVertexWriter.h"
#include "textureAttrib.h"
#include "transparencyAttrib.h"
#include "colorAttrib.h"
#include "renderState.h"
#include "config_gobj.h"

TypeHandle DynamicTextGlyph::_type_handle;

/**
 *
 */
DynamicTextGlyph::
~DynamicTextGlyph() {
}

/**
 * Returns a pointer to the first byte in the pixel buffer associated with the
 * leftmost pixel in the indicated row, where 0 is the topmost row and _y_size
 * - _margin * 2 - 1 is the bottommost row.
 */
unsigned char *DynamicTextGlyph::
get_row(int y) {
  nassertr(y >= 0 && y < _y_size - _margin * 2, nullptr);
  nassertr(_page != nullptr, nullptr);

  // First, offset y by the glyph's start.
  y += _y + _margin;
  // Also, get the x start.
  int x = _x + _margin;

  // Invert y.
  y = _page->get_y_size() - 1 - y;

  int offset = (y * _page->get_x_size()) + x;
  int pixel_width = _page->get_num_components() * _page->get_component_width();

  return _page->modify_ram_image() + offset * pixel_width;
}

/**
 * Erases the glyph from the texture map.
 */
void DynamicTextGlyph::
erase(DynamicTextFont *font) {
  nassertv(_page != nullptr);
  nassertv(_page->has_ram_image());

  // The glyph covers the pixels from (_x, _y) over the rectangle (_x_size,
  // _y_size), but it doesn't include _margin pixels around the interior of
  // the rectangle.  Erase all the pixels that the glyph covers.
  _page->fill_region(_x + _margin,
                     _page->get_y_size() - (_y + _y_size - _margin),
                     _x_size - _margin * 2, _y_size - _margin * 2,
                     font->get_bg());
}

/**
 * Returns true if this glyph represents invisible whitespace, or false if it
 * corresponds to some visible character.
 */
bool DynamicTextGlyph::
is_whitespace() const {
  return (_page == nullptr);
}

#endif  // HAVE_FREETYPE
