/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmTextMaker.cxx
 * @author drose
 * @date 2002-04-03
 */

#include "pnmTextMaker.h"
#include "pnmTextGlyph.h"
#include "filename.h"
#include "pnmImage.h"

#include FT_OUTLINE_H

using std::wstring;

/**
 * The constructor expects the name of some font file that FreeType can read,
 * along with face_index, indicating which font within the file to load
 * (usually 0).
 */
PNMTextMaker::
PNMTextMaker(const Filename &font_filename, int face_index) {
  initialize();
  _is_valid = load_font(font_filename, face_index);
}

/**
 * This constructor works as above, but it takes the font data from an in-
 * memory buffer instead of from a named file.
 */
PNMTextMaker::
PNMTextMaker(const char *font_data, int data_length, int face_index) {
  initialize();
  _is_valid = load_font(font_data, data_length, face_index);
}

/**
 *
 */
PNMTextMaker::
PNMTextMaker(const PNMTextMaker &copy) :
  FreetypeFont(copy),
  _is_valid(copy._is_valid),
  _align(copy._align),
  _interior_flag(copy._interior_flag),
  _fg(copy._fg),
  _interior(copy._interior),
  _distance_field_radius(copy._distance_field_radius)
{
}

/**
 *
 */
PNMTextMaker::
PNMTextMaker(const FreetypeFont &copy) :
  FreetypeFont(copy),
  _is_valid(true)
{
  initialize();
}

/**
 *
 */
PNMTextMaker::
~PNMTextMaker() {
  empty_cache();
}

/**
 * Generates a single line of text into the indicated image at the indicated
 * position; the return value is the total width in pixels.
 */
int PNMTextMaker::
generate_into(const wstring &text, PNMImage &dest_image, int x, int y) {
  // First, measure the total width in pixels.
  int width = calc_width(text);

  int xp = x;
  int yp = y;

  switch (_align) {
  case A_left:
    xp = x;
    break;

  case A_center:
    xp = x - (width / 2);
    break;

  case A_right:
    xp = x - width;
    break;
  }

  // Now place the text.
  wstring::const_iterator ti;
  for (ti = text.begin(); ti != text.end(); ++ti) {
    int ch = (*ti);
    PNMTextGlyph *glyph = get_glyph(ch);
    if (_interior_flag) {
      glyph->place(dest_image, xp, yp, _fg, _interior);
    } else {
      glyph->place(dest_image, xp, yp, _fg);
    }
    xp += glyph->get_advance();
  }

  return width;
}

/**
 * Returns the width in pixels of the indicated line of text.
 */
int PNMTextMaker::
calc_width(const wstring &text) {
  int width = 0;
  wstring::const_iterator ti;
  for (ti = text.begin(); ti != text.end(); ++ti) {
    int ch = (*ti);
    PNMTextGlyph *glyph = get_glyph(ch);
    width += glyph->get_advance();
  }
  return width;
}

/**
 * Returns the glyph for the indicated index, or NULL if it is not defined in
 * the font.
 */
PNMTextGlyph *PNMTextMaker::
get_glyph(int character) {
  FT_Face face = acquire_face();
  int glyph_index = FT_Get_Char_Index(face, character);
  release_face(face);

  Glyphs::iterator gi;
  gi = _glyphs.find(glyph_index);
  if (gi != _glyphs.end()) {
    return (*gi).second;
  }

  PNMTextGlyph *glyph = make_glyph(glyph_index);
  _glyphs.insert(Glyphs::value_type(glyph_index, glyph));
  return glyph;
}

/**
 * Called from both constructors to set up some initial values.
 */
void PNMTextMaker::
initialize() {
  _align = A_left;
  _interior_flag = false;
  _fg.set(0.0f, 0.0f, 0.0f, 1.0f);
  _interior.set(0.5f, 0.5f, 0.5f, 1.0f);
  _distance_field_radius = 0;
}

/**
 * Creates a new PNMTextGlyph object for the indicated index, if possible.
 */
PNMTextGlyph *PNMTextMaker::
make_glyph(int glyph_index) {
  FT_Face face = acquire_face();
  if (!load_glyph(face, glyph_index)) {
    release_face(face);
    return nullptr;
  }

  FT_GlyphSlot slot = face->glyph;

  FT_Bitmap &bitmap = slot->bitmap;

  double advance = slot->advance.x / 64.0;

  PNMTextGlyph *glyph = new PNMTextGlyph(advance);

  if (bitmap.width == 0 || bitmap.rows == 0) {
    // If we got an empty bitmap, it's a special case.
    glyph->rescale(_scale_factor);

  } else {
    PNMImage &glyph_image = glyph->_image;

    if (_distance_field_radius != 0) {
      // Ask FreeType to extract the contours out of the outline description.
      decompose_outline(slot->outline);

      PN_stdfloat tex_x_size, tex_y_size, tex_x_orig, tex_y_orig;
      FT_BBox bounds;

      // Calculate suitable texture dimensions for the signed distance field.
      // This is the same calculation that Freetype uses in its bitmap
      // renderer.
      FT_Outline_Get_CBox(&slot->outline, &bounds);

      bounds.xMin = bounds.xMin & ~63;
      bounds.yMin = bounds.yMin & ~63;
      bounds.xMax = (bounds.xMax + 63) & ~63;
      bounds.yMax = (bounds.yMax + 63) & ~63;

      tex_x_size = (bounds.xMax - bounds.xMin) >> 6;
      tex_y_size = (bounds.yMax - bounds.yMin) >> 6;
      tex_x_orig = (bounds.xMin >> 6);
      tex_y_orig = (bounds.yMax >> 6);

      if (tex_x_size == 0 || tex_y_size == 0) {
        // If we got an empty bitmap, it's a special case.
      } else {
        int outline = 0;

        int int_x_size = (int)ceil(tex_x_size);
        int int_y_size = (int)ceil(tex_y_size);

        outline = _distance_field_radius;
        int_x_size += outline * 2;
        int_y_size += outline * 2;

        glyph_image.clear(int_x_size, int_y_size, 1);
        render_distance_field(glyph_image, outline, bounds.xMin, bounds.yMin);

        glyph->_top = tex_y_orig + outline * _scale_factor;
        glyph->_left = tex_x_orig + outline * _scale_factor;

      }
    } else {
      glyph_image.clear(bitmap.width, bitmap.rows, 3);
      copy_bitmap_to_pnmimage(bitmap, glyph_image);

      glyph->_top = slot->bitmap_top;
      glyph->_left = slot->bitmap_left;

      if (_interior_flag) {
        glyph->determine_interior();
      }
    }

    glyph->rescale(_scale_factor);
  }

  release_face(face);
  return glyph;
}

/**
 * Empties the cache of previously-generated glyphs.
 */
void PNMTextMaker::
empty_cache() {
  Glyphs::iterator gi;
  for (gi = _glyphs.begin(); gi != _glyphs.end(); ++gi) {
    PNMTextGlyph *glyph = (*gi).second;
    delete glyph;
  }
}
