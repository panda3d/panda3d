// Filename: dynamicTextFont.cxx
// Created by:  drose (08Feb02)
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

#include "dynamicTextFont.h"

#ifdef HAVE_FREETYPE

#undef interface  // I don't know where this symbol is defined, but it interferes with FreeType.
#include FT_OUTLINE_H 
#ifdef FT_BBOX_H
#include FT_BBOX_H
#endif
#ifdef FT_BITMAP_H
#include FT_BITMAP_H
#endif
#ifdef FT_STROKER_H
#include FT_STROKER_H
#endif

#include "config_text.h"
#include "config_util.h"
#include "config_express.h"
#include "virtualFileSystem.h"
#include "geomVertexData.h"
#include "geomVertexFormat.h"
#include "geomVertexWriter.h"
#include "geomLinestrips.h"
#include "geomTriangles.h"
#include "renderState.h"
#include "string_utils.h"
#include "triangulator.h"
#include "nurbsCurveEvaluator.h"
#include "nurbsCurveResult.h"
//#include "renderModeAttrib.h"
//#include "antialiasAttrib.h"

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

  _fg.set(1.0f, 1.0f, 1.0f, 1.0f);
  _bg.set(1.0f, 1.0f, 1.0f, 0.0f);
  _outline_color.set(1.0f, 1.0f, 1.0f, 0.0f);
  _outline_width = 0.0f;
  _outline_feather = 0.0f;
  _has_outline = false;
  _tex_format = Texture::F_alpha;
  _needs_image_processing = false;
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

  _fg.set(1.0f, 1.0f, 1.0f, 1.0f);
  _bg.set(1.0f, 1.0f, 1.0f, 0.0f);
  _outline_color.set(1.0f, 1.0f, 1.0f, 0.0f);
  _outline_width = 0.0f;
  _outline_feather = 0.0f;
  _has_outline = false;
  _tex_format = Texture::F_alpha;
  _needs_image_processing = false;
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::Copy Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
DynamicTextFont::
DynamicTextFont(const DynamicTextFont &copy) :
  TextFont(copy),
  FreetypeFont(copy),
  _texture_margin(copy._texture_margin),
  _poly_margin(copy._poly_margin),
  _page_x_size(copy._page_x_size),
  _page_y_size(copy._page_y_size),
  _minfilter(copy._minfilter),
  _magfilter(copy._magfilter),
  _anisotropic_degree(copy._anisotropic_degree),
  _render_mode(copy._render_mode),
  _winding_order(copy._winding_order),
  _fg(copy._fg),
  _bg(copy._bg),
  _outline_color(copy._outline_color),
  _outline_width(copy._outline_width),
  _outline_feather(copy._outline_feather),
  _has_outline(copy._has_outline),
  _tex_format(copy._tex_format),
  _needs_image_processing(copy._needs_image_processing),
  _preferred_page(0)
{
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
//     Function: DynamicTextFont::make_copy
//       Access: Published
//  Description: Returns a new copy of the same font.
////////////////////////////////////////////////////////////////////
PT(TextFont) DynamicTextFont::
make_copy() const {
  return new DynamicTextFont(*this);
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
    if (glyph == (DynamicTextGlyph *)NULL || glyph->_geom_count != 0) {
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
    page->garbage_collect(this);
  }

  return removed_count;
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

    FT_Face face = acquire_face();
    if (FT_HAS_GLYPH_NAMES(face)) {
      int error = FT_Get_Glyph_Name(face, glyph_index, 
                                    glyph_name, max_glyph_name);

      // Some fonts, notably MS Mincho, claim to have glyph names but
      // only report ".notdef" as the name of each glyph.  Thanks.
      if (!error && strcmp(glyph_name, ".notdef") != 0) {
        out << " (" << glyph_name << ")";
      }
    }
    release_face(face);

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

  FT_Face face = acquire_face();
  int glyph_index = FT_Get_Char_Index(face, character);
  if (text_cat.is_spam()) {
    text_cat.spam()
      << *this << " maps " << character << " to glyph " << glyph_index << "\n";
  }

  Cache::iterator ci = _cache.find(glyph_index);
  if (ci != _cache.end()) {
    glyph = (*ci).second;
  } else {
    DynamicTextGlyph *dynamic_glyph = make_glyph(character, face, glyph_index);
    _cache.insert(Cache::value_type(glyph_index, dynamic_glyph));
    glyph = dynamic_glyph;
  }

  if (glyph == (DynamicTextGlyph *)NULL) {
    glyph = get_invalid_glyph();
    glyph_index = 0;
  }
    
  release_face(face);
  return (glyph_index != 0);
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
  _page_x_size = text_page_size[0];
  _page_y_size = text_page_size[1];

  // We don't necessarily want to use mipmaps, since we don't want to
  // regenerate those every time the texture changes, but we probably
  // do want at least linear filtering.  Use whatever the Configrc
  // file suggests.
  _minfilter = text_minfilter;
  _magfilter = text_magfilter;

  // Anisotropic filtering can help the look of the text, and doesn't
  // require generating mipmaps, but does require hardware support.
  _anisotropic_degree = text_anisotropic_degree;

  _render_mode = text_render_mode;
  _winding_order = WO_default;

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
//     Function: DynamicTextFont::determine_tex_format
//       Access: Private
//  Description: Examines the _fg, _bg, and _outline colors to
//               determine the appropriate format for the font pages,
//               including the outline properties.
////////////////////////////////////////////////////////////////////
void DynamicTextFont::
determine_tex_format() {
  nassertv(get_num_pages() == 0);

  _has_outline = (_outline_color != _bg && _outline_width > 0.0f);
  _needs_image_processing = true;

  bool needs_color = false;
  bool needs_grayscale = false;
  bool needs_alpha = false;

  if (_fg[1] != _fg[0] || _fg[2] != _fg[0] ||
      _bg[1] != _bg[0] || _bg[2] != _bg[0] ||
      (_has_outline && (_outline_color[1] != _outline_color[0] || _outline_color[2] != _outline_color[0]))) {
    // At least one of fg, bg, or outline contains a color, not just a
    // grayscale value.
    needs_color = true;

  } else if (_fg[0] != 1.0f || _fg[1] != 1.0f || _fg[2] != 1.0f ||
             _bg[0] != 1.0f || _bg[1] != 1.0f || _bg[2] != 1.0f ||
             (_has_outline && (_outline_color[0] != 1.0f || _outline_color[1] != 1.0f || _outline_color[2] != 1.0f))) {
    // fg, bg, and outline contain non-white grayscale values.
    needs_grayscale = true;
  }

  if (_fg[3] != 1.0f || _bg[3] != 1.0f || 
      (_has_outline && (_outline_color[3] != 1.0f))) {
    // fg, bg, and outline contain non-opaque alpha values.
    needs_alpha = true;
  }

  if (needs_color) {
    if (needs_alpha) {
      _tex_format = Texture::F_rgba;
    } else {
      _tex_format = Texture::F_rgb;
    }
  } else if (needs_grayscale) {
    if (needs_alpha) {
      _tex_format = Texture::F_luminance_alpha;
    } else {
      _tex_format = Texture::F_luminance;
    }
  } else {
    if (needs_alpha) {
      _tex_format = Texture::F_alpha;

      if (!_has_outline && 
          _fg == LColor(1.0f, 1.0f, 1.0f, 1.0f) &&
          _bg == LColor(1.0f, 1.0f, 1.0f, 0.0f)) {
        // This is the standard font color.  It can be copied directly
        // without any need for special processing.
        _needs_image_processing = false;
      }

    } else {
      // This won't be a very interesting font.
      _tex_format = Texture::F_luminance;
    }
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
make_glyph(int character, FT_Face face, int glyph_index) {
  if (!load_glyph(face, glyph_index, false)) {
    return (DynamicTextGlyph *)NULL;
  }

  FT_GlyphSlot slot = face->glyph;
  FT_Bitmap &bitmap = slot->bitmap;

  if ((bitmap.width == 0 || bitmap.rows == 0) && (glyph_index == 0)) {
    // Here's a special case: a glyph_index of 0 means an invalid
    // glyph.  Some fonts define a symbol to represent an invalid
    // glyph, but if that symbol is the empty bitmap, we return NULL,
    // and use Panda's invalid glyph in its place.  We do this to
    // guarantee that every invalid glyph is visible as *something*.
    return NULL;
  }

  PN_stdfloat advance = slot->advance.x / 64.0;

  if (_render_mode != RM_texture && 
      slot->format == ft_glyph_format_outline) {
    // Re-stroke the glyph to make it an outline glyph.
    /*
    FT_Stroker stroker;
    FT_Stroker_New(face->memory, &stroker);
    FT_Stroker_Set(stroker, 16 * 16, FT_STROKER_LINECAP_BUTT,
                   FT_STROKER_LINEJOIN_ROUND, 0);

    FT_Stroker_ParseOutline(stroker, &slot->outline, 0);

    FT_UInt num_points, num_contours;
    FT_Stroker_GetCounts(stroker, &num_points, &num_contours);

    FT_Outline border;
    FT_Outline_New(_ft_library, num_points, num_contours, &border);
    border.n_points = 0;
    border.n_contours = 0;
    FT_Stroker_Export(stroker, &border);
    FT_Stroker_Done(stroker);

    FT_Outline_Done(_ft_library, &slot->outline);
    memcpy(&slot->outline, &border, sizeof(border));
    */

    // Ask FreeType to extract the contours out of the outline
    // description.
    FT_Outline_Funcs funcs;
    memset(&funcs, 0, sizeof(funcs));
    funcs.move_to = (FT_Outline_MoveTo_Func)outline_move_to;
    funcs.line_to = (FT_Outline_LineTo_Func)outline_line_to;
    funcs.conic_to = (FT_Outline_ConicTo_Func)outline_conic_to;
    funcs.cubic_to = (FT_Outline_CubicTo_Func)outline_cubic_to;

    WindingOrder wo = _winding_order;
    if (wo == WO_default) {
      // If we weren't told an explicit winding order, ask FreeType to
      // figure it out.  Sometimes it appears to guess wrong.
#ifdef FT_ORIENTATION_FILL_RIGHT
      if (FT_Outline_Get_Orientation(&slot->outline) == FT_ORIENTATION_FILL_RIGHT) {
        wo = WO_right;
      } else {
        wo = WO_left;
      }
#else
      // Hmm.  Assign a right-winding (TTF) orientation if FreeType
      // can't tell us.
      wo = WO_right;
#endif  // FT_ORIENTATION_FILL_RIGHT
    }

    if (wo != WO_left) {
      FT_Outline_Reverse(&slot->outline);
    }

    _contours.clear();
    FT_Outline_Decompose(&slot->outline, &funcs, (void *)this);

    PT(DynamicTextGlyph) glyph = 
      new DynamicTextGlyph(character, advance / _font_pixels_per_unit);
    switch (_render_mode) {
    case RM_wireframe:
      render_wireframe_contours(glyph);
      return glyph;

    case RM_polygon:
      render_polygon_contours(glyph, true, false);
      return glyph;

    case RM_extruded:
      render_polygon_contours(glyph, false, true);
      return glyph;

    case RM_solid:
      render_polygon_contours(glyph, true, true);
      return glyph;

    case RM_texture:
    default:
      break;
    }
  }

  // Render the glyph if necessary.
  if (slot->format != ft_glyph_format_bitmap) {
    FT_Render_Glyph(slot, ft_render_mode_normal);
  }

  if (bitmap.width == 0 || bitmap.rows == 0) {
    // If we got an empty bitmap, it's a special case.

    PT(DynamicTextGlyph) glyph = 
      new DynamicTextGlyph(character, advance / _font_pixels_per_unit);
    _empty_glyphs.push_back(glyph);
    return glyph;

  } else {
    DynamicTextGlyph *glyph;

    PN_stdfloat tex_x_size = bitmap.width;
    PN_stdfloat tex_y_size = bitmap.rows;

    int outline = 0;

    if (_tex_pixels_per_unit == _font_pixels_per_unit &&
        !_needs_image_processing) {
      // If the bitmap produced from the font doesn't require scaling
      // or any other processing before it goes to the texture, we can
      // just copy it directly into the texture.
      glyph = slot_glyph(character, bitmap.width, bitmap.rows);
      copy_bitmap_to_texture(bitmap, glyph);

    } else {
      // Otherwise, we need to copy to a PNMImage first, so we can
      // scale it and/or process it; and then copy it to the texture
      // from there.
      tex_x_size /= _scale_factor;
      tex_y_size /= _scale_factor;
      int int_x_size = (int)ceil(tex_x_size);
      int int_y_size = (int)ceil(tex_y_size);
      int bmp_x_size = (int)(int_x_size * _scale_factor + 0.5f);
      int bmp_y_size = (int)(int_y_size * _scale_factor + 0.5f);

      PNMImage image(bmp_x_size, bmp_y_size, PNMImage::CT_grayscale);
      copy_bitmap_to_pnmimage(bitmap, image);

      PNMImage reduced(int_x_size, int_y_size, PNMImage::CT_grayscale);
      reduced.quick_filter_from(image);

      // convert the outline width from points to tex_pixels.
      PN_stdfloat outline_pixels = _outline_width / _points_per_unit * _tex_pixels_per_unit;
      outline = (int)ceil(outline_pixels);

      int_x_size += outline * 2;
      int_y_size += outline * 2;
      tex_x_size += outline * 2;
      tex_y_size += outline * 2;
      glyph = slot_glyph(character, int_x_size, int_y_size);

      if (outline != 0) {
        // Pad the glyph image to make room for the outline.
        PNMImage padded(int_x_size, int_y_size, PNMImage::CT_grayscale);
        padded.copy_sub_image(reduced, outline, outline);
        copy_pnmimage_to_texture(padded, glyph);

      } else {
        copy_pnmimage_to_texture(reduced, glyph);
      }
    }

    glyph->make_geom((int)floor(slot->bitmap_top + outline * _scale_factor + 0.5f),
                     (int)floor(slot->bitmap_left - outline * _scale_factor + 0.5f),
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
  if (!_needs_image_processing) {
    // Copy the image directly into the alpha component of the
    // texture.
    nassertv(glyph->_page->get_num_components() == 1);
    for (int yi = 0; yi < image.get_y_size(); yi++) {
      unsigned char *texture_row = glyph->get_row(yi);
      nassertv(texture_row != (unsigned char *)NULL);
      for (int xi = 0; xi < image.get_x_size(); xi++) {
        texture_row[xi] = image.get_gray_val(xi, yi);
      }
    }

  } else {
    if (_has_outline) {
      // Gaussian blur the glyph to generate an outline.
      PNMImage outline(image.get_x_size(), image.get_y_size(), PNMImage::CT_grayscale);
      PN_stdfloat outline_pixels = _outline_width / _points_per_unit * _tex_pixels_per_unit;
      outline.gaussian_filter_from(outline_pixels * 0.707, image);

      // Filter the resulting outline to make a harder edge.  Square
      // _outline_feather first to make the range more visually linear
      // (this approximately compensates for the Gaussian falloff of
      // the feathered edge).
      PN_stdfloat f = _outline_feather * _outline_feather;

      for (int yi = 0; yi < outline.get_y_size(); yi++) {
        for (int xi = 0; xi < outline.get_x_size(); xi++) {
          PN_stdfloat v = outline.get_gray(xi, yi);
          if (v == 0.0f) {
            // Do nothing.
          } else if (v >= f) {
            // Clamp to 1.
            outline.set_gray(xi, yi, 1.0);
          } else {
            // Linearly scale the range 0 .. f onto 0 .. 1.
            outline.set_gray(xi, yi, v / f);
          }
        }
      }

      // Now blend that into the texture.
      blend_pnmimage_to_texture(outline, glyph, _outline_color);
    }

    // Colorize the image as we copy it in.  This assumes the previous
    // color at this part of the texture was already initialized to
    // the background color.
    blend_pnmimage_to_texture(image, glyph, _fg);
  }    
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::blend_pnmimage_to_texture
//       Access: Private
//  Description: Blends the PNMImage into the appropriate part of the
//               texture, where 0.0 in the image indicates the color
//               remains the same, and 1.0 indicates the color is
//               assigned the indicated foreground color.
////////////////////////////////////////////////////////////////////
void DynamicTextFont::
blend_pnmimage_to_texture(const PNMImage &image, DynamicTextGlyph *glyph,
                          const LColor &fg) {
  LColor fgv = fg * 255.0f;

  int num_components = glyph->_page->get_num_components();
  if (num_components == 1) {
    // Luminance or alpha.
    int ci = 3;
    if (glyph->_page->get_format() != Texture::F_alpha) {
      ci = 0;
    }

    for (int yi = 0; yi < image.get_y_size(); yi++) {
      unsigned char *texture_row = glyph->get_row(yi);
      nassertv(texture_row != (unsigned char *)NULL);
      for (int xi = 0; xi < image.get_x_size(); xi++) {
        unsigned char *tr = texture_row + xi;
        PN_stdfloat t = (PN_stdfloat)image.get_gray(xi, yi);
        tr[0] = (unsigned char)(tr[0] + t * (fgv[ci] - tr[0]));
      }
    }

  } else if (num_components == 2) {
    // Luminance + alpha.

    for (int yi = 0; yi < image.get_y_size(); yi++) {
      unsigned char *texture_row = glyph->get_row(yi);
      nassertv(texture_row != (unsigned char *)NULL);
      for (int xi = 0; xi < image.get_x_size(); xi++) {
        unsigned char *tr = texture_row + xi * 2;
        PN_stdfloat t = (PN_stdfloat)image.get_gray(xi, yi);
        tr[0] = (unsigned char)(tr[0] + t * (fgv[0] - tr[0]));
        tr[1] = (unsigned char)(tr[1] + t * (fgv[3] - tr[1]));
      }
    }

  } else if (num_components == 3) {
    // RGB.

    for (int yi = 0; yi < image.get_y_size(); yi++) {
      unsigned char *texture_row = glyph->get_row(yi);
      nassertv(texture_row != (unsigned char *)NULL);
      for (int xi = 0; xi < image.get_x_size(); xi++) {
        unsigned char *tr = texture_row + xi * 3;
        PN_stdfloat t = (PN_stdfloat)image.get_gray(xi, yi);
        tr[0] = (unsigned char)(tr[0] + t * (fgv[2] - tr[0]));
        tr[1] = (unsigned char)(tr[1] + t * (fgv[1] - tr[1]));
        tr[2] = (unsigned char)(tr[2] + t * (fgv[0] - tr[2]));
      }
    }

  } else { // (num_components == 4)
    // RGBA.

    for (int yi = 0; yi < image.get_y_size(); yi++) {
      unsigned char *texture_row = glyph->get_row(yi);
      nassertv(texture_row != (unsigned char *)NULL);
      for (int xi = 0; xi < image.get_x_size(); xi++) {
        unsigned char *tr = texture_row + xi * 4;
        PN_stdfloat t = (PN_stdfloat)image.get_gray(xi, yi);
        tr[0] = (unsigned char)(tr[0] + t * (fgv[2] - tr[0]));
        tr[1] = (unsigned char)(tr[1] + t * (fgv[1] - tr[1]));
        tr[2] = (unsigned char)(tr[2] + t * (fgv[0] - tr[2]));
        tr[3] = (unsigned char)(tr[3] + t * (fgv[3] - tr[3]));
      }
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
slot_glyph(int character, int x_size, int y_size) {
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
      DynamicTextGlyph *glyph = page->slot_glyph(character, x_size, y_size, _texture_margin);
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
    return slot_glyph(character, x_size, y_size);

  } else {
    // No good; all recorded glyphs are actually in use.  We need to
    // make a new page.
    _preferred_page = _pages.size();
    PT(DynamicTextPage) page = new DynamicTextPage(this, _preferred_page);
    _pages.push_back(page);
    return page->slot_glyph(character, x_size, y_size, _texture_margin);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::render_wireframe_contours
//       Access: Private
//  Description: Converts from the _contours list to an actual glyph
//               geometry, as a wireframe render.
////////////////////////////////////////////////////////////////////
void DynamicTextFont::
render_wireframe_contours(DynamicTextGlyph *glyph) {
  PT(GeomVertexData) vdata = new GeomVertexData
    (string(), GeomVertexFormat::get_v3(),
     Geom::UH_static);
  GeomVertexWriter vertex(vdata, InternalName::get_vertex());

  PT(GeomLinestrips) lines = new GeomLinestrips(Geom::UH_static);

  Contours::const_iterator ci;
  for (ci = _contours.begin(); ci != _contours.end(); ++ci) {
    const Contour &contour = (*ci);
    Points::const_iterator pi;

    for (pi = contour._points.begin(); pi != contour._points.end(); ++pi) {
      const LPoint2 &p = (*pi)._p;
      vertex.add_data3(p[0], 0.0f, p[1]);
    }

    lines->add_next_vertices(contour._points.size());
    lines->close_primitive();
  }

  glyph->set_geom(vdata, lines, RenderState::make_empty());
  _contours.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::render_polygon_contours
//       Access: Private
//  Description: Converts from the _contours list to an actual glyph
//               geometry, as a polygon render.
////////////////////////////////////////////////////////////////////
void DynamicTextFont::
render_polygon_contours(DynamicTextGlyph *glyph, bool face, bool extrude) {
  PT(GeomVertexData) vdata = new GeomVertexData
    (string(), GeomVertexFormat::get_v3n3(),
     Geom::UH_static);
  GeomVertexWriter vertex(vdata, InternalName::get_vertex());
  GeomVertexWriter normal(vdata, InternalName::get_normal());

  PT(GeomTriangles) tris = new GeomTriangles(Geom::UH_static);
  Triangulator t;

  Contours::iterator ci;

  if (face) {
    // First, build up the list of vertices for the face, and
    // determine which contours are solid and which are holes.
    for (ci = _contours.begin(); ci != _contours.end(); ++ci) {
      Contour &contour = (*ci);
      
      t.clear_polygon();
      contour._start_vertex = t.get_num_vertices();
      for (size_t i = 0; i < contour._points.size() - 1; ++i) {
        const LPoint2 &p = contour._points[i]._p;
        vertex.add_data3(p[0], 0.0f, p[1]);
        normal.add_data3(0.0f, -1.0f, 0.0f);
        int vi = t.add_vertex(p[0], p[1]);
        t.add_polygon_vertex(vi);
      }
      
      contour._is_solid = t.is_left_winding();
    }

    // Now go back and generate the actual triangles for the face.
    for (ci = _contours.begin(); ci != _contours.end(); ++ci) {
      const Contour &contour = (*ci);
      
      if (contour._is_solid && !contour._points.empty()) {
        t.clear_polygon();
        for (size_t i = 0; i < contour._points.size() - 1; ++i) {
          t.add_polygon_vertex(contour._start_vertex + i);
        }
        
        // Also add all the holes to each polygon.
        Contours::iterator cj;
        for (cj = _contours.begin(); cj != _contours.end(); ++cj) {
          Contour &hole = (*cj);
          if (!hole._is_solid && !hole._points.empty()) {
            t.begin_hole();
            for (size_t j = 0; j < hole._points.size() - 1; ++j) {
              t.add_hole_vertex(hole._start_vertex + j);
            }
          }
        }
        
        t.triangulate();
        int num_triangles = t.get_num_triangles();
        for (int ti = 0; ti < num_triangles; ++ti) {
          tris->add_vertex(t.get_triangle_v0(ti));
          tris->add_vertex(t.get_triangle_v1(ti));
          tris->add_vertex(t.get_triangle_v2(ti));
          tris->close_primitive();
        }
      }
    }
  }

  if (extrude) {
    // If we're generating extruded geometry (polygons along the
    // edges, down the y axis), generate them now.  These are pretty
    // easy, but we need to create more vertices--they don't share the
    // same normals.
    for (ci = _contours.begin(); ci != _contours.end(); ++ci) {
      const Contour &contour = (*ci);
      Points::const_iterator pi;

      for (size_t i = 0; i < contour._points.size(); ++i) {
        const ContourPoint &cp = contour._points[i];
        const LPoint2 &p = cp._p;
        const LVector2 &t_in = cp._in;
        const LVector2 &t_out = cp._out;

        LVector3 n_in(t_in[1], 0.0f, -t_in[0]);
        vertex.add_data3(p[0], 1.0f, p[1]);
        vertex.add_data3(p[0], 0.0f, p[1]);
        normal.add_data3(n_in);
        normal.add_data3(n_in);

        if (i != 0) {
          int vi = vertex.get_write_row();
          tris->add_vertex(vi - 4);
          tris->add_vertex(vi - 2);
          tris->add_vertex(vi - 1);
          tris->close_primitive();
          tris->add_vertex(vi - 1);
          tris->add_vertex(vi - 3);
          tris->add_vertex(vi - 4);
          tris->close_primitive();
        }

        if (i != contour._points.size() - 1 && !t_in.almost_equal(t_out)) {
          // If the out tangent is different from the in tangent, we
          // need to store new vertices for the next quad.
          LVector3 n_out(t_out[1], 0.0f, -t_out[0]);
          vertex.add_data3(p[0], 1.0f, p[1]);
          vertex.add_data3(p[0], 0.0f, p[1]);
          normal.add_data3(n_out);
          normal.add_data3(n_out);
        }
      }
    }

    if (face) {
      // Render the back side of the face too.
      int back_start = vertex.get_write_row();

      for (ci = _contours.begin(); ci != _contours.end(); ++ci) {
        Contour &contour = (*ci);
        for (size_t i = 0; i < contour._points.size() - 1; ++i) {
          const LPoint2 &p = contour._points[i]._p;
          vertex.add_data3(p[0], 1.0f, p[1]);
          normal.add_data3(0.0f, 1.0f, 0.0f);
        }
      }

      // Now go back and generate the actual triangles for the face.
      for (ci = _contours.begin(); ci != _contours.end(); ++ci) {
        const Contour &contour = (*ci);
      
        if (contour._is_solid && !contour._points.empty()) {
          t.clear_polygon();
          for (size_t i = 0; i < contour._points.size() - 1; ++i) {
            t.add_polygon_vertex(contour._start_vertex + i);
          }
        
          // Also add all the holes to each polygon.
          Contours::iterator cj;
          for (cj = _contours.begin(); cj != _contours.end(); ++cj) {
            Contour &hole = (*cj);
            if (!hole._is_solid && !hole._points.empty()) {
              t.begin_hole();
              for (size_t j = 0; j < hole._points.size() - 1; ++j) {
                t.add_hole_vertex(hole._start_vertex + j);
              }
            }
          }
        
          t.triangulate();
          int num_triangles = t.get_num_triangles();
          for (int ti = 0; ti < num_triangles; ++ti) {
            tris->add_vertex(t.get_triangle_v2(ti) + back_start);
            tris->add_vertex(t.get_triangle_v1(ti) + back_start);
            tris->add_vertex(t.get_triangle_v0(ti) + back_start);
            tris->close_primitive();
          }
        }
      }
    }
  }

  glyph->set_geom(vdata, tris, RenderState::make_empty());
  //  glyph->set_geom(vdata, tris, RenderState::make(RenderModeAttrib::make(RenderModeAttrib::M_wireframe)));
  //  glyph->set_geom(vdata, tris, RenderState::make(AntialiasAttrib::make(AntialiasAttrib::M_auto)));
  _contours.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::outline_move_to
//       Access: Private, Static
//  Description: A callback from FT_Outline_Decompose().  It marks the
//               beginning of a new contour.
////////////////////////////////////////////////////////////////////
int DynamicTextFont::
outline_move_to(const FT_Vector *to, void *user) {
  DynamicTextFont *self = (DynamicTextFont *)user;

  // Convert from 26.6 pixel units to Panda units.
  PN_stdfloat scale = 1.0f / (64.0f * self->_font_pixels_per_unit);
  LPoint2 p = LPoint2(to->x, to->y) * scale;

  if (self->_contours.empty() ||
      !self->_contours.back()._points.empty()) {
    self->_contours.push_back(Contour());
  }
  self->_q = p;
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::outline_line_to
//       Access: Private, Static
//  Description: A callback from FT_Outline_Decompose().  It marks a
//               straight line in the contour.
////////////////////////////////////////////////////////////////////
int DynamicTextFont::
outline_line_to(const FT_Vector *to, void *user) {
  DynamicTextFont *self = (DynamicTextFont *)user;
  nassertr(!self->_contours.empty(), 1);

  // Convert from 26.6 pixel units to Panda units.
  PN_stdfloat scale = 1.0f / (64.0f * self->_font_pixels_per_unit);
  LPoint2 p = LPoint2(to->x, to->y) * scale;

  // Compute the tangent: this is just the vector from the last point.
  LVector2 t = (p - self->_q);
  t.normalize();

  if (self->_contours.back()._points.empty()) {
    self->_contours.back()._points.push_back(ContourPoint(self->_q, LVector2::zero(), t));
  } else {
    self->_contours.back()._points.back().connect_to(t);
  }
  
  self->_contours.back()._points.push_back(ContourPoint(p, t, LVector2::zero()));
  self->_q = p;
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::outline_conic_to
//       Access: Private, Static
//  Description: A callback from FT_Outline_Decompose().  It marks a
//               parabolic (3rd-order) Bezier curve in the contour.
////////////////////////////////////////////////////////////////////
int DynamicTextFont::
outline_conic_to(const FT_Vector *control,
                 const FT_Vector *to, void *user) {
  DynamicTextFont *self = (DynamicTextFont *)user;
  nassertr(!self->_contours.empty(), 1);

  // Convert from 26.6 pixel units to Panda units.
  PN_stdfloat scale = 1.0f / (64.0f * self->_font_pixels_per_unit);

  LPoint2 c = LPoint2(control->x, control->y) * scale;
  LPoint2 p = LPoint2(to->x, to->y) * scale;

  // The NurbsCurveEvaluator will evaluate the Bezier segment for us.
  NurbsCurveEvaluator nce;
  nce.local_object();
  nce.set_order(3);
  nce.reset(3);
  nce.set_vertex(0, LVecBase3(self->_q[0], self->_q[1], 0.0f));
  nce.set_vertex(1, LVecBase3(c[0], c[1], 0.0f));
  nce.set_vertex(2, LVecBase3(p[0], p[1], 0.0f));

  self->_q = p;

  PT(NurbsCurveResult) ncr = nce.evaluate();
  return self->outline_nurbs(ncr);
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::outline_cubic_to
//       Access: Private, Static
//  Description: A callback from FT_Outline_Decompose().  It marks a
//               cubic (4th-order) Bezier curve in the contour.
////////////////////////////////////////////////////////////////////
int DynamicTextFont::
outline_cubic_to(const FT_Vector *control1, const FT_Vector *control2,
                 const FT_Vector *to, void *user) {
  DynamicTextFont *self = (DynamicTextFont *)user;
  nassertr(!self->_contours.empty(), 1);

  // Convert from 26.6 pixel units to Panda units.
  PN_stdfloat scale = 1.0f / (64.0f * self->_font_pixels_per_unit);

  LPoint2 c1 = LPoint2(control1->x, control1->y) * scale;
  LPoint2 c2 = LPoint2(control2->x, control2->y) * scale;
  LPoint2 p = LPoint2(to->x, to->y) * scale;

  // The NurbsCurveEvaluator will evaluate the Bezier segment for us.
  NurbsCurveEvaluator nce;
  nce.local_object();
  nce.set_order(4);
  nce.reset(4);
  nce.set_vertex(0, LVecBase3(self->_q[0], self->_q[1], 0.0f));
  nce.set_vertex(1, LVecBase3(c1[0], c1[1], 0.0f));
  nce.set_vertex(2, LVecBase3(c2[0], c2[1], 0.0f));
  nce.set_vertex(3, LVecBase3(p[0], p[1], 0.0f));

  self->_q = p;

  PT(NurbsCurveResult) ncr = nce.evaluate();
  return self->outline_nurbs(ncr);
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::outline_nurbs
//       Access: Private
//  Description: Called internally by outline_cubic_to() and
//               outline_conic_to().
////////////////////////////////////////////////////////////////////
int DynamicTextFont::
outline_nurbs(NurbsCurveResult *ncr) {
  // Sample it down so that the lines approximate the curve to within
  // a "pixel."
  ncr->adaptive_sample(1.0f / _font_pixels_per_unit);

  int num_samples = ncr->get_num_samples();

  bool needs_connect = false;
  int start = 1;
  if (_contours.back()._points.empty()) {
    // If we haven't got the first point of this contour yet, we must
    // add it now.
    start = 0;
  } else {
    needs_connect = true;
  }

  for (int i = start; i < num_samples; ++i) {
    PN_stdfloat st = ncr->get_sample_t(i);
    const LPoint3 &p = ncr->get_sample_point(i);

    PN_stdfloat st0 = st, st1 = st;
    if (i > 0) {
      st0 = ncr->get_sample_t(i - 1) * 0.1f + st * 0.9f;
    }
    if (i < num_samples - 1) {
      st1 = ncr->get_sample_t(i + 1) * 0.1f + st * 0.9f;
    }
    // Compute the tangent by deltaing nearby points.  Don't evaluate
    // the tangent from the NURBS, since that doesn't appear to be
    // reliable.
    LPoint3 p0, p1;
    ncr->eval_point(st0, p0);
    ncr->eval_point(st1, p1);
    LVector3 t = p1 - p0;
    t.normalize();

    if (needs_connect) {
      _contours.back()._points.back().connect_to(LVector2(t[0], t[1]));
      needs_connect = false;
    }

    _contours.back()._points.push_back(ContourPoint(p[0], p[1], t[0], t[1]));
  }

  return 0;
}

#endif  // HAVE_FREETYPE
