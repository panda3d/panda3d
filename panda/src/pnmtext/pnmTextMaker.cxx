// Filename: textMaker.cxx
// Created by:  drose (03Apr02)
//
//////////////////////////////////////////////////////////////////////
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

#include "pnmTextMaker.h"
#include "pnmTextGlyph.h"
#include "filename.h"
#include "pnmImage.h"

////////////////////////////////////////////////////////////////////
//     Function: PNMTextMaker::Constructor
//       Access: Public
//  Description: The constructor expects the name of some font file
//               that FreeType can read, along with face_index,
//               indicating which font within the file to load
//               (usually 0).
////////////////////////////////////////////////////////////////////
PNMTextMaker::
PNMTextMaker(const Filename &font_filename, int face_index) {
  initialize();
  _is_valid = load_font(font_filename, face_index);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMTextMaker::Constructor
//       Access: Public
//  Description: This constructor works as above, but it takes the
//               font data from an in-memory buffer instead of from a
//               named file.
////////////////////////////////////////////////////////////////////
PNMTextMaker::
PNMTextMaker(const char *font_data, int data_length, int face_index) {
  initialize();
  _is_valid = load_font(font_data, data_length, face_index);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMTextMaker::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PNMTextMaker::
~PNMTextMaker() {
  empty_cache();
}

////////////////////////////////////////////////////////////////////
//     Function: PNMTextMaker::generate_into
//       Access: Public
//  Description: Generates text into the indicated image at the
//               indicated position.
////////////////////////////////////////////////////////////////////
void PNMTextMaker::
generate_into(const wstring &text, PNMImage &dest_image, int x, int y) {
  // First, measure the total width in pixels.
  int width = 0;
  wstring::const_iterator ti;
  for (ti = text.begin(); ti != text.end(); ++ti) {
    int ch = (*ti);
    PNMTextGlyph *glyph = get_glyph(ch);
    width += glyph->get_advance();
  }

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
}

////////////////////////////////////////////////////////////////////
//     Function: PNMTextMaker::get_glyph
//       Access: Public
//  Description: Returns the glyph for the indicated index, or NULL if
//               it is not defined in the font.
////////////////////////////////////////////////////////////////////
PNMTextGlyph *PNMTextMaker::
get_glyph(int character) {
  int glyph_index = FT_Get_Char_Index(_face, character);

  Glyphs::iterator gi;
  gi = _glyphs.find(glyph_index);
  if (gi != _glyphs.end()) {
    return (*gi).second;
  }

  PNMTextGlyph *glyph = make_glyph(glyph_index);
  _glyphs.insert(Glyphs::value_type(glyph_index, glyph));
  return glyph;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMTextMaker::initialize
//       Access: Private
//  Description: Called from both constructors to set up some initial
//               values.
////////////////////////////////////////////////////////////////////
void PNMTextMaker::
initialize() {
  _align = A_left;
  _interior_flag = false;
  _fg.set(0.0f, 0.0f, 0.0f, 1.0f);
  _interior.set(0.5f, 0.5f, 0.5f, 1.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMTextMaker::make_glyph
//       Access: Private
//  Description: Creates a new PNMTextGlyph object for the indicated
//               index, if possible.
////////////////////////////////////////////////////////////////////
PNMTextGlyph *PNMTextMaker::
make_glyph(int glyph_index) {
  if (!load_glyph(glyph_index)) {
    return (PNMTextGlyph *)NULL;
  }

  FT_GlyphSlot slot = _face->glyph;
  FT_Bitmap &bitmap = slot->bitmap;

  double advance = slot->advance.x / 64.0;

  if (bitmap.width == 0 || bitmap.rows == 0) {
    // If we got an empty bitmap, it's a special case.
    PNMTextGlyph *glyph = new PNMTextGlyph(advance);
    glyph->rescale(_scale_factor);
    return glyph;

  } else {
    PNMTextGlyph *glyph = new PNMTextGlyph(advance);
    PNMImage &glyph_image = glyph->_image;
    glyph_image.clear(bitmap.width, bitmap.rows, 3);
    copy_bitmap_to_pnmimage(bitmap, glyph_image);

    glyph->_top = slot->bitmap_top;
    glyph->_left = slot->bitmap_left;

    if (_interior_flag) {
      glyph->determine_interior();
    }
    glyph->rescale(_scale_factor);
    return glyph;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMTextMaker::empty_cache
//       Access: Private
//  Description: Empties the cache of previously-generated glyphs.
////////////////////////////////////////////////////////////////////
void PNMTextMaker::
empty_cache() {
  Glyphs::iterator gi;
  for (gi = _glyphs.begin(); gi != _glyphs.end(); ++gi) {
    PNMTextGlyph *glyph = (*gi).second;
    delete glyph;
  }
}
