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
#include "ctype.h"

bool DynamicTextFont::_update_cleared_glyphs = text_update_cleared_glyphs;

FT_Library DynamicTextFont::_ft_library;
bool DynamicTextFont::_ft_initialized = false;
bool DynamicTextFont::_ft_ok = false;

TypeHandle DynamicTextFont::_type_handle;


// This constant determines how big a particular point size font
// appears to be.  By convention, 10 points is 1 unit (e.g. 1 foot)
// high.
static const float points_per_unit = 10.0f;

// A universal typographic convention.
static const float points_per_inch = 72.0f;

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
  _texture_margin = text_texture_margin;
  _poly_margin = text_poly_margin;
  _page_x_size = text_page_x_size;
  _page_y_size = text_page_y_size;
  _point_size = text_point_size;
  _pixels_per_unit = text_pixels_per_unit;
  _small_caps = text_small_caps;
  _small_caps_scale = text_small_caps_scale;

  _preferred_page = 0;

  if (!_ft_initialized) {
    initialize_ft_library();
  }
  if (!_ft_ok) {
    text_cat.error()
      << "Unable to read font " << font_filename
      << ": FreeType library not initialized properly.\n";
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
      if (_face->style_name != NULL) {
        name += " ";
        name += _face->style_name;
      }
      set_name(name);

      text_cat.info()
        << "Loaded font " << get_name() << "\n";
      _is_valid = true;
      reset_scale();
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
//               if it does not.
////////////////////////////////////////////////////////////////////
bool DynamicTextFont::
get_glyph(int character, const TextGlyph *&glyph, float &glyph_scale) {
  if (!_is_valid) {
    return false;
  }

  glyph_scale = 1.0f;
  if (character < 128 && islower(character) && get_small_caps()) {
    // If we have small_caps on, we implement lowercase letters by
    // applying a scale to the corresponding uppercase letter.
    glyph_scale = get_small_caps_scale();
    character = toupper(character);
  }

  int glyph_index = FT_Get_Char_Index(_face, character);

  Cache::iterator ci = _cache.find(glyph_index);
  if (ci != _cache.end()) {
    glyph = (*ci).second;
  } else {
    DynamicTextGlyph *dynamic_glyph = make_glyph(glyph_index);
    _cache.insert(Cache::value_type(glyph_index, dynamic_glyph));
    glyph = dynamic_glyph;
  }

  return (glyph != (DynamicTextGlyph *)NULL);
}
 
////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::reset_scale
//       Access: Private
//  Description: Resets the font to use the current _point_size and
//               _pixels_per_unit.  Returns true if successful, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool DynamicTextFont::
reset_scale() {
  float units_per_inch = (points_per_inch / points_per_unit);
  int dpi = (int)(_pixels_per_unit * units_per_inch);
  
  int error = FT_Set_Char_Size(_face,
                               (int)(_point_size * 64), (int)(_point_size * 64),
                               dpi, dpi);
  if (error) {
    text_cat.warning()
      << "Unable to set " << get_name() 
      << " to " << _point_size << "pt at " << dpi << " dpi.\n";
    _line_height = 1.0f;
    return false;
  }

  // The face's height is only relevant for scalable fonts,
  // according to FreeType.  How should we determine whether we
  // have a scalable font or otherwise?
  float pixel_size = _point_size * (_pixels_per_unit / points_per_unit);
  _line_height = (float)_face->height * pixel_size / ((float)_face->units_per_EM * 64.0f);

  // Determine the correct width for a space.
  error = FT_Load_Char(_face, ' ', FT_LOAD_DEFAULT);
  if (error) {
    // Space isn't defined.  Oh well.
    _space_advance = 0.25f * _line_height;

  } else {
    _space_advance = _face->glyph->advance.x / (_pixels_per_unit * 64.0f);
  }

  return true;
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
  int error = FT_Load_Glyph(_face, glyph_index, FT_LOAD_RENDER);
  if (error) {
    text_cat.error()
      << "Unable to render glyph " << glyph_index << "\n";
    return (DynamicTextGlyph *)NULL;
  }

  FT_GlyphSlot slot = _face->glyph;
  FT_Bitmap &bitmap = slot->bitmap;

  float advance = slot->advance.x / 64.0;

  if (bitmap.width == 0 || bitmap.rows == 0) {
    // If we got an empty bitmap, it's a special case.
    PT(DynamicTextGlyph) glyph = new DynamicTextGlyph(advance / _pixels_per_unit);
    _empty_glyphs.push_back(glyph);
    return glyph;

  } else {
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
    
    glyph->make_geom(slot->bitmap_top, slot->bitmap_left, advance,
                     _poly_margin, _pixels_per_unit);
    return glyph;
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
    PT(DynamicTextPage) page = new DynamicTextPage(this);
    _pages.push_back(page);
    return page->slot_glyph(x_size, y_size, _texture_margin);
  }
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
