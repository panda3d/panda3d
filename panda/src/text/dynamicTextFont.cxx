// Filename: dynamicTextFont.cxx
// Created by:  drose (08Feb02)
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

#include "dynamicTextFont.h"

#ifdef HAVE_FREETYPE

#include "config_text.h"
#include "config_util.h"

FT_Library DynamicTextFont::_ft_library;
bool DynamicTextFont::_ft_initialized = false;
bool DynamicTextFont::_ft_ok = false;

TypeHandle DynamicTextFont::_type_handle;


// This constant determines how big a particular point size font
// appears.  By convention, 10 points is 1 foot high.
static const float points_per_unit = 10.0f;

// A universal convention.
static const float points_per_inch = 72.0f;

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::Constructor
//       Access: Published
//  Description: The constructor expects the name of some font file
//               that FreeType can read, along with face_index,
//               indicating which font within the file to load
//               (usually 0), the point size of the font, and the
//               resolution at which to generate the font.  
//
//               The choice of point size affects the apparent size of
//               the generated characters (as well as the clarity),
//               while the pixels_per_unit affects only the clarity.
////////////////////////////////////////////////////////////////////
DynamicTextFont::
DynamicTextFont(const Filename &font_filename, int face_index,
                float point_size, float pixels_per_unit) {
  _margin = 2;
  _page_x_size = 256;
  _page_y_size = 256;
  _pixels_per_unit = pixels_per_unit;

  float units_per_inch = (points_per_inch / points_per_unit);
  int dpi = (int)(_pixels_per_unit * units_per_inch);

  if (!_ft_initialized) {
    initialize_ft_library();
  }
  if (!_ft_ok) {
    text_cat.error()
      << "Unable to read font " << font_filename << ": FreeType library not available.\n";
    return;
  }

  Filename path(font_filename);
  if (!path.resolve_filename(get_model_path())) {
    text_cat.error()
      << "Unable to find font file " << font_filename << "\n";
  } else {
    string os_specific = path.to_os_specific();
    
    int error = FT_New_Face(_ft_library,
                            os_specific.c_str(),
                            face_index,
                            &_face);
    if (error == FT_Err_Unknown_File_Format) {
      text_cat.error()
        << "Unable to read font " << font_filename << ": unknown file format.\n";
    } else if (error) {
      text_cat.error()
        << "Unable to read font " << font_filename << ": invalid.\n";

    } else {
      string name = _face->family_name;
      name += " ";
      name += _face->style_name;

      _is_valid = true;
      set_name(name);

      text_cat.info()
        << "Loaded font " << get_name() << "\n";

      error = FT_Set_Char_Size(_face,
                               (int)(point_size * 64), (int)(point_size * 64),
                               dpi, dpi);
      if (error) {
        text_cat.warning()
          << "Unable to set point size of " << get_name() 
          << " to " << point_size << " at " << dpi << " dots per inch.\n";
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::get_num_pages
//       Access: Published
//  Description: Returns the number of pages associated with the font.
//               Initially, the font has zero pages; when the first
//               piece of text is rendered with the font, it will add
//               additional pages as needed.  Each page is a Texture
//               object that contains the images for each of the
//               glyphs currently in use somewhere.
////////////////////////////////////////////////////////////////////
int DynamicTextFont::
get_num_pages() const {
  return _pages.size();
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::get_page
//       Access: Published
//  Description: Returns the nth page associated with the font.
//               Initially, the font has zero pages; when the first
//               piece of text is rendered with the font, it will add
//               additional pages as needed.  Each page is a Texture
//               object that contains the images for each of the
//               glyphs currently in use somewhere.
////////////////////////////////////////////////////////////////////
DynamicTextPage *DynamicTextFont::
get_page(int n) const {
  nassertr(n >= 0 && n < (int)_pages.size(), (DynamicTextPage *)NULL);
  return _pages[n];
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::write
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DynamicTextFont::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << "DynamicTextFont " << get_name() << ".\n";
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::get_glyph
//       Access: Public, Virtual
//  Description: Returns the glyph associated with the given character
//               code, or NULL if there is no such glyph.
////////////////////////////////////////////////////////////////////
const TextGlyph *DynamicTextFont::
get_glyph(int character) {
  Cache::iterator ci = _cache.find(character);
  if (ci != _cache.end()) {
    return (*ci).second;
  }
  DynamicTextGlyph *glyph = make_glyph(character);
  _cache.insert(Cache::value_type(character, glyph));
  return glyph;
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::make_glyph
//       Access: Private
//  Description: Slots a space in the texture map for the new
//               character and renders the glyph, returning the
//               newly-created TextGlyph object, or NULL if the
//               glyph cannot be created for some reason.
////////////////////////////////////////////////////////////////////
DynamicTextGlyph *DynamicTextFont::
make_glyph(int character) {
  int error = FT_Load_Char(_face, character, FT_LOAD_RENDER);
  if (error) {
    text_cat.error()
      << "Unable to render character " << character << "\n";
    return (DynamicTextGlyph *)NULL;
  }

  FT_GlyphSlot slot = _face->glyph;
  FT_Bitmap &bitmap = slot->bitmap;

  if (bitmap.pixel_mode != ft_pixel_mode_grays) {
    text_cat.error()
      << "Unexpected pixel mode in bitmap: " << (int)bitmap.pixel_mode << "\n";
    return (DynamicTextGlyph *)NULL;
  }

  if (bitmap.num_grays != 256) {
    // We expect 256 levels of grayscale to come back from FreeType,
    // since that's what we asked for.
    text_cat.warning()
      << "Expected 256 levels of gray, got " << bitmap.num_grays << "\n";
  }

  DynamicTextGlyph *glyph = slot_glyph(bitmap.width, bitmap.rows);

  // Now copy the rendered glyph into the texture.
  unsigned char *buffer_row = bitmap.buffer;
  for (int yi = 0; yi < bitmap.rows; yi++) {
    unsigned char *texture_row = glyph->get_row(yi);
    nassertr(texture_row != (unsigned char *)NULL, (DynamicTextGlyph *)NULL);
    memcpy(texture_row, buffer_row, bitmap.width);
    buffer_row += bitmap.pitch;
  }
  glyph->_page->mark_dirty(Texture::DF_image);

  float advance = slot->advance.x / 64.0;
  glyph->make_geom(slot->bitmap_top, slot->bitmap_left, advance,
                   _pixels_per_unit);
  return glyph;
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::slot_glyph
//       Access: Private
//  Description: Chooses a page that will have room for a glyph of the
//               indicated size (after expanding the indicated size by
//               the current margin).  Returns the newly-allocated
//               glyph on the chosen page; the glyph has not been
//               filled in yet except with its size.
////////////////////////////////////////////////////////////////////
DynamicTextGlyph *DynamicTextFont::
slot_glyph(int x_size, int y_size) {
  // Increase the indicated size by the current margin.
  x_size += _margin * 2;
  y_size += _margin * 2;

  Pages::iterator pi;
  for (pi = _pages.begin(); pi != _pages.end(); ++pi) {
    DynamicTextPage *page = (*pi);

    DynamicTextGlyph *glyph = page->slot_glyph(x_size, y_size, _margin);
    if (glyph != (DynamicTextGlyph *)NULL) {
      return glyph;
    }

    if (page->is_empty()) {
      // If we couldn't even put in on an empty page, we're screwed.
      text_cat.error()
        << "Glyph of size " << x_size << " by " << y_size
        << " won't fit on an empty page.\n";
      return (DynamicTextGlyph *)NULL;
    }
  }

  // We need to make a new page.
  PT(DynamicTextPage) page = new DynamicTextPage(this);
  _pages.push_back(page);
  return page->slot_glyph(x_size, y_size, _margin);
}


////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::initialize_ft_library
//       Access: Private, Static
//  Description: Should be called exactly once to initialize the
//               FreeType library.
////////////////////////////////////////////////////////////////////
void DynamicTextFont::
initialize_ft_library() {
  if (!_ft_initialized) {
    int error = FT_Init_FreeType(&_ft_library);
    _ft_initialized = true;
    if (error) {
      text_cat.error()
        << "Unable to initialize FreeType; DynamicTextFonts will not load.\n";
    } else {
      _ft_ok = true;
    }
  }
}

#endif  // HAVE_FREETYPE
