// Filename: dynamicTextFont.cxx
// Created by:  drose (08Feb02)
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

#include "dynamicTextFont.h"

#ifdef HAVE_FREETYPE

#include "config_text.h"
#include "config_util.h"
#include "config_express.h"
#include "virtualFileSystem.h"

bool DynamicTextFont::_update_cleared_glyphs = text_update_cleared_glyphs;

TypeHandle DynamicTextFont::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::Constructor
//       Access: Published
//  Description: The constructor expects the name of some font file
//               that FreeType can read, along with face_index,
//               indicating which font within the file to load
//               (usually 0).
////////////////////////////////////////////////////////////////////
DynamicTextFont::
DynamicTextFont(const Filename &font_filename, int face_index) {
  initialize();
  _is_valid = load_font(font_filename, face_index);
  TextFont::set_name(FreetypeFont::get_name());
  TextFont::_line_height = FreetypeFont::get_line_height();
  TextFont::_space_advance = FreetypeFont::get_space_advance();
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::Constructor
//       Access: Published
//  Description: This constructor accepts a table of data representing
//               the font file, loaded from some source other than a
//               filename on disk.
////////////////////////////////////////////////////////////////////
DynamicTextFont::
DynamicTextFont(const char *font_data, int data_length, int face_index) {
  initialize();
  _is_valid = load_font(font_data, data_length, face_index);
  TextFont::set_name(FreetypeFont::get_name());
  TextFont::_line_height = FreetypeFont::_line_height;
  TextFont::_space_advance = FreetypeFont::_space_advance;
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
DynamicTextFont::
~DynamicTextFont() {
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
//     Function: DynamicTextFont::garbage_collect
//       Access: Published
//  Description: Removes all of the glyphs from the font that are no
//               longer being used by any Geoms.  Returns the number
//               of glyphs removed.
////////////////////////////////////////////////////////////////////
int DynamicTextFont::
garbage_collect() {
  int removed_count = 0;

  // First, remove all the old entries from our cache index.
  Cache new_cache;
  Cache::iterator ci;
  for (ci = _cache.begin(); ci != _cache.end(); ++ci) {
    DynamicTextGlyph *glyph = (*ci).second;
    if (glyph->_geom_count != 0) {
      // Keep this one.
      new_cache.insert(new_cache.end(), (*ci));
    } else {
      // Drop this one.
      removed_count++;
    }
  }
  _cache.swap(new_cache);

  // Now, go through each page and do the same thing.
  Pages::iterator pi;
  for (pi = _pages.begin(); pi != _pages.end(); ++pi) {
    DynamicTextPage *page = (*pi);
    page->garbage_collect();
  }

  return removed_count;
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::update_texture_memory
//       Access: Published
//  Description: Marks all of the pages dirty so they will be reloaded
//               into texture memory.  This is necessary only if
//               set_update_cleared_glyphs() is false, and some
//               textures have recently been removed from the pages
//               (for instance, after a call to garbage_collect()).
//
//               Calling this just ensures that what you see when you
//               apply the texture page to a polygon represents what
//               is actually stored on the page.
////////////////////////////////////////////////////////////////////
void DynamicTextFont::
update_texture_memory() {
  Pages::iterator pi;
  for (pi = _pages.begin(); pi != _pages.end(); ++pi) {
    DynamicTextPage *page = (*pi);
    page->mark_dirty(Texture::DF_image);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::clear
//       Access: Published
//  Description: Drops all the glyphs out of the cache and frees any
//               association with any previously-generated pages.
//
//               Calling this frequently can result in wasted texture
//               memory, as any previously rendered text will still
//               keep a pointer to the old, previously-generated
//               pages.  As long as the previously rendered text
//               remains around, the old pages will also remain
//               around.
////////////////////////////////////////////////////////////////////
void DynamicTextFont::
clear() {
  _cache.clear();
  _pages.clear();
  _empty_glyphs.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::write
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DynamicTextFont::
write(ostream &out, int indent_level) const {
  static const int max_glyph_name = 1024;
  char glyph_name[max_glyph_name];

  indent(out, indent_level)
    << "DynamicTextFont " << get_name() << ", " 
    << get_num_pages() << " pages, "
    << _cache.size() << " glyphs:\n";
  Cache::const_iterator ci;
  for (ci = _cache.begin(); ci != _cache.end(); ++ci) {
    int glyph_index = (*ci).first;
    DynamicTextGlyph *glyph = (*ci).second;
    indent(out, indent_level + 2) 
      << glyph_index;

    if (FT_HAS_GLYPH_NAMES(_face)) {
      int error = FT_Get_Glyph_Name(_face, glyph_index, 
                                    glyph_name, max_glyph_name);

      // Some fonts, notably MS Mincho, claim to have glyph names but
      // only report ".notdef" as the name of each glyph.  Thanks.
      if (!error && strcmp(glyph_name, ".notdef") != 0) {
        out << " (" << glyph_name << ")";
      }
    }

    out << ", count = " << glyph->_geom_count << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::get_glyph
//       Access: Public, Virtual
//  Description: Gets the glyph associated with the given character
//               code, as well as an optional scaling parameter that
//               should be applied to the glyph's geometry and advance
//               parameters.  Returns true if the glyph exists, false
//               if it does not.  Even if the return value is false,
//               the value for glyph might be filled in with a
//               printable glyph.
////////////////////////////////////////////////////////////////////
bool DynamicTextFont::
get_glyph(int character, const TextGlyph *&glyph) {
  if (!_is_valid) {
    glyph = (TextGlyph *)NULL;
    return false;
  }

  int glyph_index = FT_Get_Char_Index(_face, character);
  if (text_cat.is_spam()) {
    text_cat.spam()
      << *this << " maps " << character << " to glyph " << glyph_index << "\n";
  }

  Cache::iterator ci = _cache.find(glyph_index);
  if (ci != _cache.end()) {
    glyph = (*ci).second;
  } else {
    DynamicTextGlyph *dynamic_glyph = make_glyph(glyph_index);
    _cache.insert(Cache::value_type(glyph_index, dynamic_glyph));
    glyph = dynamic_glyph;
  }

  return (glyph_index != 0 && glyph != (DynamicTextGlyph *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::initialize
//       Access: Private
//  Description: Called from both constructors to set up some initial
//               values.
////////////////////////////////////////////////////////////////////
void DynamicTextFont::
initialize() {
  _texture_margin = text_texture_margin;
  _poly_margin = text_poly_margin;
  _page_x_size = text_page_x_size;
  _page_y_size = text_page_y_size;

  // We don't necessarily want to use mipmaps, since we don't want to
  // regenerate those every time the texture changes, but we probably
  // do want at least linear filtering.  Use whatever the Configrc
  // file suggests.
  _minfilter = text_minfilter;
  _magfilter = text_magfilter;

  // Anisotropic filtering can help the look of the text, and doesn't
  // require generating mipmaps, but does require hardware support.
  _anisotropic_degree = text_anisotropic_degree;

  _preferred_page = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::update_filters
//       Access: Private
//  Description: Reapplies all current filter settings to all of the
//               pages.  This is normally called whenever the filter
//               settings change.
////////////////////////////////////////////////////////////////////
void DynamicTextFont::
update_filters() {
  Pages::iterator pi;
  for (pi = _pages.begin(); pi != _pages.end(); ++pi) {
    DynamicTextPage *page = (*pi);
    page->set_minfilter(_minfilter);
    page->set_magfilter(_magfilter);
    page->set_anisotropic_degree(_anisotropic_degree);
  }
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
make_glyph(int glyph_index) {
  if (!load_glyph(glyph_index)) {
    return (DynamicTextGlyph *)NULL;
  }

  FT_GlyphSlot slot = _face->glyph;
  FT_Bitmap &bitmap = slot->bitmap;

  float advance = slot->advance.x / 64.0;

  if (bitmap.width == 0 || bitmap.rows == 0) {
    // If we got an empty bitmap, it's a special case.
    PT(DynamicTextGlyph) glyph = 
      new DynamicTextGlyph(advance / _font_pixels_per_unit);
    _empty_glyphs.push_back(glyph);
    return glyph;

  } else {
    DynamicTextGlyph *glyph;

    float tex_x_size = bitmap.width;
    float tex_y_size = bitmap.rows;

    if (_tex_pixels_per_unit == _font_pixels_per_unit) {
      // If the bitmap produced from the font doesn't require scaling
      // before it goes to the texture, we can just copy it directly
      // into the texture.
      glyph = slot_glyph(bitmap.width, bitmap.rows);
      copy_bitmap_to_texture(bitmap, glyph);

    } else {
      // Otherwise, we need to copy to a PNMImage first, so we can
      // scale it; and then copy it to the texture from there.
      tex_x_size /= _scale_factor;
      tex_y_size /= _scale_factor;
      int int_x_size = (int)ceil(tex_x_size);
      int int_y_size = (int)ceil(tex_y_size);
      int bmp_x_size = (int)(int_x_size * _scale_factor + 0.5f);
      int bmp_y_size = (int)(int_y_size * _scale_factor + 0.5f);
      glyph = slot_glyph(int_x_size, int_y_size);
      
      PNMImage image(bmp_x_size, bmp_y_size, PNMImage::CT_grayscale);
      copy_bitmap_to_pnmimage(bitmap, image);

      PNMImage reduced(int_x_size, int_y_size, PNMImage::CT_grayscale);
      reduced.quick_filter_from(image);
      copy_pnmimage_to_texture(reduced, glyph);
    }
      
    glyph->_page->mark_dirty(Texture::DF_image);
    
    glyph->make_geom(slot->bitmap_top, slot->bitmap_left,
                     advance, _poly_margin,
                     tex_x_size, tex_y_size,
                     _font_pixels_per_unit, _tex_pixels_per_unit);
    return glyph;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::copy_bitmap_to_texture
//       Access: Private
//  Description: Copies a bitmap as rendered by FreeType directly into
//               the texture memory image for the indicated glyph,
//               without any scaling of pixels.
////////////////////////////////////////////////////////////////////
void DynamicTextFont::
copy_bitmap_to_texture(const FT_Bitmap &bitmap, DynamicTextGlyph *glyph) {
  if (bitmap.pixel_mode == ft_pixel_mode_grays && bitmap.num_grays == 256) {
    // This is the easy case: we can memcpy the rendered glyph
    // directly into our texture image, one row at a time.
    unsigned char *buffer_row = bitmap.buffer;
    for (int yi = 0; yi < bitmap.rows; yi++) {
      
      unsigned char *texture_row = glyph->get_row(yi);
      nassertv(texture_row != (unsigned char *)NULL);
      memcpy(texture_row, buffer_row, bitmap.width);
      buffer_row += bitmap.pitch;
    }
    
  } else if (bitmap.pixel_mode == ft_pixel_mode_mono) {
    // This is a little bit more work: we have to expand the
    // one-bit-per-pixel bitmap into a one-byte-per-pixel texture.
    unsigned char *buffer_row = bitmap.buffer;
    for (int yi = 0; yi < bitmap.rows; yi++) {
      unsigned char *texture_row = glyph->get_row(yi);
      nassertv(texture_row != (unsigned char *)NULL);
      
      int bit = 0x80;
      unsigned char *b = buffer_row;
      for (int xi = 0; xi < bitmap.width; xi++) {
        if (*b & bit) {
          texture_row[xi] = 0xff;
        } else {
          texture_row[xi] = 0x00;
        }
        bit >>= 1;
        if (bit == 0) {
          ++b;
          bit = 0x80;
        }
      }
      
      buffer_row += bitmap.pitch;
    }
    
    
  } else if (bitmap.pixel_mode == ft_pixel_mode_grays) {
    // Here we must expand a grayscale pixmap with n levels of gray
    // into our 256-level texture.
    unsigned char *buffer_row = bitmap.buffer;
    for (int yi = 0; yi < bitmap.rows; yi++) {
      unsigned char *texture_row = glyph->get_row(yi);
      nassertv(texture_row != (unsigned char *)NULL);
      for (int xi = 0; xi < bitmap.width; xi++) {
        texture_row[xi] = (int)(buffer_row[xi] * 255) / (bitmap.num_grays - 1);
      }
      buffer_row += bitmap.pitch;
    }
    
  } else {
    text_cat.error()
      << "Unexpected pixel mode in bitmap: " << (int)bitmap.pixel_mode << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::copy_pnmimage_to_texture
//       Access: Private
//  Description: Copies a bitmap stored in a PNMImage into
//               the texture memory image for the indicated glyph.
////////////////////////////////////////////////////////////////////
void DynamicTextFont::
copy_pnmimage_to_texture(const PNMImage &image, DynamicTextGlyph *glyph) {
  for (int yi = 0; yi < image.get_y_size(); yi++) {
    unsigned char *texture_row = glyph->get_row(yi);
    nassertv(texture_row != (unsigned char *)NULL);
    for (int xi = 0; xi < image.get_x_size(); xi++) {
      texture_row[xi] = image.get_gray_val(xi, yi);
    }
  }
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
  x_size += _texture_margin * 2;
  y_size += _texture_margin * 2;

  if (!_pages.empty()) {
    // Start searching on the preferred page.  That way, we'll fill up
    // the preferred page first, and we can gradually rotate this page
    // around; it keeps us from spending too much time checking
    // already-filled pages for space.
    _preferred_page = _preferred_page % _pages.size();
    int pi = _preferred_page;

    do {
      DynamicTextPage *page = _pages[pi];
      DynamicTextGlyph *glyph = page->slot_glyph(x_size, y_size, _texture_margin);
      if (glyph != (DynamicTextGlyph *)NULL) {
        // Once we found a page to hold the glyph, that becomes our
        // new preferred page.
        _preferred_page = pi;
        return glyph;
      }

      if (page->is_empty()) {
        // If we couldn't even put it on an empty page, we're screwed.
        text_cat.error()
          << "Glyph of size " << x_size << " by " << y_size
          << " pixels won't fit on an empty page.\n";
        return (DynamicTextGlyph *)NULL;
      }

      pi = (pi + 1) % _pages.size();
    } while (pi != _preferred_page);
  }

  // All pages are filled.  Can we free up space by removing some old
  // glyphs?
  if (garbage_collect() != 0) {
    // Yes, we just freed up some space.  Try once more, recursively.
    return slot_glyph(x_size, y_size);

  } else {
    // No good; all recorded glyphs are actually in use.  We need to
    // make a new page.
    _preferred_page = _pages.size();
    PT(DynamicTextPage) page = new DynamicTextPage(this, _preferred_page);
    _pages.push_back(page);
    return page->slot_glyph(x_size, y_size, _texture_margin);
  }
}

#endif  // HAVE_FREETYPE
