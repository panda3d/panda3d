// Filename: dynamicTextPage.cxx
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

#include "dynamicTextPage.h"
#include "dynamicTextFont.h"
#include "config_text.h"

#ifdef HAVE_FREETYPE


TypeHandle DynamicTextPage::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextPage::Constructor
//       Access: Publiic
//  Description: 
////////////////////////////////////////////////////////////////////
DynamicTextPage::
DynamicTextPage(DynamicTextFont *font, int page_number) : 
  _font(font)
{
  // Since the texture might change frequently, don't try to compress
  // it by default.
  set_compression(CM_off);

  // It's usually pretty important for text to look its best, and it
  // doesn't usually have a high fill factor.
  set_quality_level(text_quality_level);

  _x_size = _font->get_page_x_size();
  _y_size = _font->get_page_y_size();

  setup_2d_texture(_x_size, _y_size, T_unsigned_byte, font->get_tex_format());

  // Assign a name to the Texture.
  ostringstream strm;
  strm << font->get_name() << "_" << page_number;
  set_name(strm.str());

  // We'd better never free this image.
  set_keep_ram_image(true);

  set_minfilter(_font->get_minfilter());
  set_magfilter(_font->get_magfilter());

  set_anisotropic_degree(_font->get_anisotropic_degree());

  // Clamp to an explicit invisible border, so we don't get bleeding
  // at the edges at all.
  set_wrap_u(text_wrap_mode);
  set_wrap_v(text_wrap_mode);
  set_border_color(font->get_bg());

  // Fill the page with the font's background color.
  fill_region(0, 0, _x_size, _y_size, font->get_bg());
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextPage::slot_glyph
//       Access: Public
//  Description: Finds space within the page for a glyph of the
//               indicated size.  If space is found, creates a new
//               glyph object and returns it; otherwise, returns NULL.
////////////////////////////////////////////////////////////////////
DynamicTextGlyph *DynamicTextPage::
slot_glyph(int character, int x_size, int y_size, int margin) {
  int x, y;
  if (!find_hole(x, y, x_size, y_size)) {
    // No room for the glyph.
    return (DynamicTextGlyph *)NULL;
  }

  // The glyph can be fit at (x, y).  Slot it.
  PT(DynamicTextGlyph) glyph = 
    new DynamicTextGlyph(character, this,
                         x, y, x_size, y_size, margin);
  _glyphs.push_back(glyph);
  return glyph;
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextPage::fill_region
//       Access: Private
//  Description: Fills a rectangular region of the texture with the
//               indicated color.
////////////////////////////////////////////////////////////////////
void DynamicTextPage::
fill_region(int x, int y, int x_size, int y_size, const LColor &color) {
  nassertv(x >= 0 && x + x_size <= _x_size && y >= 0 && y + y_size <= _y_size);
  int num_components = get_num_components();
  if (num_components == 1) {
    // Luminance or alpha.
    int ci = 3;
    if (get_format() != Texture::F_alpha) {
      ci = 0;
    }
    
    unsigned char v = (unsigned char)(color[ci] * 255.0f);

    unsigned char *image = modify_ram_image();
    for (int yi = y; yi < y + y_size; yi++) {
      unsigned char *row = image + yi * _x_size;
      memset(row + x, v, x_size);
    }

  } else if (num_components == 2) {
    // Luminance + alpha.

    union {
      unsigned char p[2];
      PN_uint16 v;
    } v;

    v.p[0] = (unsigned char)(color[0] * 255.0f);
    v.p[1] = (unsigned char)(color[3] * 255.0f);

    PN_uint16 *image = (PN_uint16 *)modify_ram_image().p();
    for (int yi = y; yi < y + y_size; yi++) {
      PN_uint16 *row = image + yi * _x_size ;
      for (int xi = x; xi < x + x_size; xi++) {
        row[xi] = v.v;
      }
    }

  } else if (num_components == 3) {
    // RGB.

    unsigned char p0 = (unsigned char)(color[2] * 255.0f);
    unsigned char p1 = (unsigned char)(color[1] * 255.0f);
    unsigned char p2 = (unsigned char)(color[0] * 255.0f);

    unsigned char *image = modify_ram_image();
    for (int yi = y; yi < y + y_size; yi++) {
      unsigned char *row = image + yi * _x_size * 3;
      for (int xi = x; xi < x + x_size; xi++) {
        row[xi * 3] = p0;
        row[xi * 3 + 1] = p1;
        row[xi * 3 + 2] = p2;
      }
    }
    
  } else { // (num_components == 4)
    // RGBA.
    union {
      unsigned char p[4];
      PN_uint32 v;
    } v;

    v.p[0] = (unsigned char)(color[2] * 255.0f);
    v.p[1] = (unsigned char)(color[1] * 255.0f);
    v.p[2] = (unsigned char)(color[0] * 255.0f);
    v.p[3] = (unsigned char)(color[3] * 255.0f);

    PN_uint32 *image = (PN_uint32 *)modify_ram_image().p();
    for (int yi = y; yi < y + y_size; yi++) {
      PN_uint32 *row = image + yi * _x_size;
      for (int xi = x; xi < x + x_size; xi++) {
        row[xi] = v.v;
      }
    }
  }    
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
garbage_collect(DynamicTextFont *font) {
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
      glyph->erase(font);
    }
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
