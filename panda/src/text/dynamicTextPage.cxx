// Filename: dynamicTextPage.cxx
// Created by:  drose (09Feb02)
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

#include "dynamicTextPage.h"
#include "dynamicTextFont.h"

#ifdef HAVE_FREETYPE


TypeHandle DynamicTextPage::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextPage::Constructor
//       Access: Publiic
//  Description: 
////////////////////////////////////////////////////////////////////
DynamicTextPage::
DynamicTextPage(DynamicTextFont *font) : 
  _font(font)
{
  _x_size = _font->get_page_x_size();
  _y_size = _font->get_page_y_size();

  // Initialize the Texture to an empty, black (transparent) image of
  // the appropriate size.
  _pbuffer = new PixelBuffer(_x_size, _y_size, 1, 1, 
                             PixelBuffer::T_unsigned_byte,
                             PixelBuffer::F_alpha);
  mark_dirty(DF_image);

  // We'd better never free this image.
  set_keep_ram_image(true);

  set_minfilter(_font->get_minfilter());
  set_magfilter(_font->get_magfilter());

  set_anisotropic_degree(_font->get_anisotropic_degree());

  // It's slightly better to let the texture clamp, rather than
  // wrapping, so we're less likely to get bleeding at the edges.
  set_wrapu(WM_clamp);
  set_wrapv(WM_clamp);
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextPage::slot_glyph
//       Access: Publiic
//  Description: Finds space within the page for a glyph of the
//               indicated size.  If space is found, creates a new
//               glyph object and returns it; otherwise, returns NULL.
////////////////////////////////////////////////////////////////////
DynamicTextGlyph *DynamicTextPage::
slot_glyph(int x_size, int y_size, int margin) {
  int x, y;
  if (!find_hole(x, y, x_size, y_size)) {
    // No room for the glyph.
    return (DynamicTextGlyph *)NULL;
  }

  // The glyph can be fit at (x, y).  Slot it.
  PT(DynamicTextGlyph) glyph = 
    new DynamicTextGlyph(this, x, y, x_size, y_size, margin);
  _glyphs.push_back(glyph);
  return glyph;
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextPage::garbage_collect
//       Access: Private
//  Description: Removes all of the glyphs from the page that are no
//               longer being used by any Geoms.  This should only be
//               called from DynamicTextFont::garbage_collect(), since
//               it is important to remove these glyphs from the
//               font's index first.
////////////////////////////////////////////////////////////////////
int DynamicTextPage::
garbage_collect() {
  int removed_count = 0;

  Glyphs new_glyphs;
  Glyphs::iterator gi;
  for (gi = _glyphs.begin(); gi != _glyphs.end(); ++gi) {
    DynamicTextGlyph *glyph = (*gi);
    if (glyph->_geom_count != 0) {
      // Keep this one.
      new_glyphs.insert(new_glyphs.end(), (*gi));
    } else {
      // Drop this one.
      removed_count++;
      glyph->erase();
    }
  }

  if (removed_count != 0 && DynamicTextFont::get_update_cleared_glyphs()) {
    // Only mark the texture dirty if the user specifically requested
    // an automatic texture memory update upon clearing glyphs.
    mark_dirty(Texture::DF_image);
  }

  _glyphs.swap(new_glyphs);
  return removed_count;
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextPage::find_hole
//       Access: Private
//  Description: Searches for a hole of at least x_size by y_size
//               pixels somewhere within the page.  If a suitable hole
//               is found, sets x and y to the top left corner and
//               returns true; otherwise, returns false.
////////////////////////////////////////////////////////////////////
bool DynamicTextPage::
find_hole(int &x, int &y, int x_size, int y_size) const {
  y = 0;
  while (y + y_size <= _y_size) {
    int next_y = _y_size;
    // Scan along the row at 'y'.
    x = 0;
    while (x + x_size <= _x_size) {
      int next_x = x;

      // Consider the spot at x, y.
      DynamicTextGlyph *overlap = find_overlap(x, y, x_size, y_size);

      if (overlap == (DynamicTextGlyph *)NULL) {
        // Hooray!
        return true;
      }

      next_x = overlap->_x + overlap->_x_size;
      next_y = min(next_y, overlap->_y + overlap->_y_size);
      nassertr(next_x > x, false);
      x = next_x;
    }

    nassertr(next_y > y, false);
    y = next_y;
  }

  // Nope, wouldn't fit anywhere.
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextPage::find_overlap
//       Access: Private
//  Description: If the rectangle whose top left corner is x, y and
//               whose size is x_size, y_size describes an empty hole
//               that does not overlap any placed glyphs, returns
//               NULL; otherwise, returns the first placed glyph
//               that the image does overlap.  It is assumed the
//               rectangle lies completely within the boundaries of
//               the page itself.
////////////////////////////////////////////////////////////////////
DynamicTextGlyph *DynamicTextPage::
find_overlap(int x, int y, int x_size, int y_size) const {
  Glyphs::const_iterator gi;
  for (gi = _glyphs.begin(); gi != _glyphs.end(); ++gi) {
    DynamicTextGlyph *glyph = (*gi);
    if (glyph->intersects(x, y, x_size, y_size)) {
      return glyph;
    }
  }

  return (DynamicTextGlyph *)NULL;
}


#endif  // HAVE_FREETYPE
