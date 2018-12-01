/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dynamicTextFont.cxx
 * @author drose
 * @date 2002-02-08
 */

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
#include "config_putil.h"
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
// #include "renderModeAttrib.h" #include "antialiasAttrib.h"
#include "colorAttrib.h"
#include "textureAttrib.h"
#include "transparencyAttrib.h"

#ifdef HAVE_HARFBUZZ
#include <hb-ft.h>
#endif

TypeHandle DynamicTextFont::_type_handle;


/**
 * The constructor expects the name of some font file that FreeType can read,
 * along with face_index, indicating which font within the file to load
 * (usually 0).
 */
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

/**
 * This constructor accepts a table of data representing the font file, loaded
 * from some source other than a filename on disk.
 */
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

/**
 *
 */
DynamicTextFont::
DynamicTextFont(const DynamicTextFont &copy) :
  TextFont(copy),
  FreetypeFont(copy),
  _texture_margin(copy._texture_margin),
  _poly_margin(copy._poly_margin),
  _page_size(copy._page_size),
  _minfilter(copy._minfilter),
  _magfilter(copy._magfilter),
  _anisotropic_degree(copy._anisotropic_degree),
  _render_mode(copy._render_mode),
  _fg(copy._fg),
  _bg(copy._bg),
  _outline_color(copy._outline_color),
  _outline_width(copy._outline_width),
  _outline_feather(copy._outline_feather),
  _has_outline(copy._has_outline),
  _tex_format(copy._tex_format),
  _needs_image_processing(copy._needs_image_processing),
  _preferred_page(0),
  _hb_font(nullptr)
{
}

/**
 *
 */
DynamicTextFont::
~DynamicTextFont() {
#ifdef HAVE_HARFBUZZ
  if (_hb_font != nullptr) {
    hb_font_destroy(_hb_font);
  }
#endif
}

/**
 * Returns a new copy of the same font.
 */
PT(TextFont) DynamicTextFont::
make_copy() const {
  return new DynamicTextFont(*this);
}

/**
 * Returns the number of pages associated with the font.  Initially, the font
 * has zero pages; when the first piece of text is rendered with the font, it
 * will add additional pages as needed.  Each page is a Texture object that
 * contains the images for each of the glyphs currently in use somewhere.
 */
int DynamicTextFont::
get_num_pages() const {
  return _pages.size();
}

/**
 * Returns the nth page associated with the font.  Initially, the font has
 * zero pages; when the first piece of text is rendered with the font, it will
 * add additional pages as needed.  Each page is a Texture object that
 * contains the images for each of the glyphs currently in use somewhere.
 */
DynamicTextPage *DynamicTextFont::
get_page(int n) const {
  nassertr(n >= 0 && n < (int)_pages.size(), nullptr);
  return _pages[n];
}

/**
 * Removes all of the glyphs from the font that are no longer being used by
 * any Geoms.  Returns the number of glyphs removed.
 */
int DynamicTextFont::
garbage_collect() {
  int removed_count = 0;

  // First, remove all the old entries from our cache index.
  Cache new_cache;
  Cache::iterator ci;
  for (ci = _cache.begin(); ci != _cache.end(); ++ci) {
    const TextGlyph *glyph = (*ci).second;
    if (glyph == nullptr || glyph->get_ref_count() > 1) {
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

/**
 * Drops all the glyphs out of the cache and frees any association with any
 * previously-generated pages.
 *
 * Calling this frequently can result in wasted texture memory, as any
 * previously rendered text will still keep a pointer to the old, previously-
 * generated pages.  As long as the previously rendered text remains around,
 * the old pages will also remain around.
 */
void DynamicTextFont::
clear() {
  _cache.clear();
  _pages.clear();
  _empty_glyphs.clear();

#ifdef HAVE_HARFBUZZ
  if (_hb_font != nullptr) {
    hb_font_destroy(_hb_font);
    _hb_font = nullptr;
  }
#endif
}

/**
 *
 */
void DynamicTextFont::
write(std::ostream &out, int indent_level) const {
  static const int max_glyph_name = 1024;
  char glyph_name[max_glyph_name];

  indent(out, indent_level)
    << "DynamicTextFont " << get_name() << ", "
    << get_num_pages() << " pages, "
    << _cache.size() << " glyphs:\n";
  Cache::const_iterator ci;
  for (ci = _cache.begin(); ci != _cache.end(); ++ci) {
    int glyph_index = (*ci).first;
    indent(out, indent_level + 2)
      << glyph_index;

    FT_Face face = acquire_face();
    if (FT_HAS_GLYPH_NAMES(face)) {
      int error = FT_Get_Glyph_Name(face, glyph_index,
                                    glyph_name, max_glyph_name);

      // Some fonts, notably MS Mincho, claim to have glyph names but only
      // report ".notdef" as the name of each glyph.  Thanks.
      if (!error && strcmp(glyph_name, ".notdef") != 0) {
        out << " (" << glyph_name << ")";
      }
    }
    release_face(face);

    out << '\n';
  }
}

/**
 * Gets the glyph associated with the given character code, as well as an
 * optional scaling parameter that should be applied to the glyph's geometry
 * and advance parameters.  Returns true if the glyph exists, false if it does
 * not.  Even if the return value is false, the value for glyph might be
 * filled in with a printable glyph.
 */
bool DynamicTextFont::
get_glyph(int character, CPT(TextGlyph) &glyph) {
  if (!_is_valid) {
    glyph = nullptr;
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
    glyph = make_glyph(character, face, glyph_index);
    _cache.insert(Cache::value_type(glyph_index, glyph.p()));
  }

  if (glyph.is_null()) {
    glyph = get_invalid_glyph();
    glyph_index = 0;
  }

  release_face(face);
  return (glyph_index != 0);
}

/**
 * Returns the amount by which to offset the second glyph when it directly
 * follows the first glyph.  This is an additional offset that is added on top
 * of the advance.
 */
PN_stdfloat DynamicTextFont::
get_kerning(int first, int second) const {
  if (!_is_valid) {
    return 0;
  }

  FT_Face face = acquire_face();
  if (!FT_HAS_KERNING(face)) {
    release_face(face);
    return 0;
  }

  int first_index = FT_Get_Char_Index(face, first);
  int second_index = FT_Get_Char_Index(face, second);

  FT_Vector delta;
  FT_Get_Kerning(face, first_index, second_index, FT_KERNING_DEFAULT, &delta);
  release_face(face);

  return delta.x / (_font_pixels_per_unit * 64);
}

/**
 * Like get_glyph, but uses a glyph index.
 */
bool DynamicTextFont::
get_glyph_by_index(int character, int glyph_index, CPT(TextGlyph) &glyph) {
  if (!_is_valid) {
    glyph = nullptr;
    return false;
  }

  Cache::iterator ci = _cache.find(glyph_index);
  if (ci != _cache.end()) {
    glyph = (*ci).second;
  } else {
    FT_Face face = acquire_face();
    glyph = make_glyph(character, face, glyph_index);
    _cache.insert(Cache::value_type(glyph_index, glyph.p()));
    release_face(face);
  }

  if (glyph.is_null()) {
    glyph = get_invalid_glyph();
    return false;
  }

  return true;
}

/**
 * If Panda was compiled with HarfBuzz enabled, returns a HarfBuzz font for
 * this font.
 */
hb_font_t *DynamicTextFont::
get_hb_font() const {
#ifdef HAVE_HARFBUZZ
  if (_hb_font != nullptr) {
    return _hb_font;
  }

  FT_Face face = acquire_face();
  _hb_font = hb_ft_font_create(face, nullptr);
  release_face(face);

  return _hb_font;
#else
  return nullptr;
#endif
}

/**
 * Called from both constructors to set up some initial values.
 */
void DynamicTextFont::
initialize() {
  _texture_margin = text_texture_margin;
  _poly_margin = text_poly_margin;
  _page_size.set(text_page_size[0], text_page_size[1]);

  // We don't necessarily want to use mipmaps, since we don't want to
  // regenerate those every time the texture changes, but we probably do want
  // at least linear filtering.  Use whatever the Configrc file suggests.
  _minfilter = text_minfilter;
  _magfilter = text_magfilter;

  // Anisotropic filtering can help the look of the text, and doesn't require
  // generating mipmaps, but does require hardware support.
  _anisotropic_degree = text_anisotropic_degree;

  _render_mode = text_render_mode;
  _winding_order = WO_default;

  _preferred_page = 0;

  _hb_font = nullptr;
}

/**
 * Reapplies all current filter settings to all of the pages.  This is
 * normally called whenever the filter settings change.
 */
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

/**
 * Examines the _fg, _bg, and _outline colors to determine the appropriate
 * format for the font pages, including the outline properties.
 */
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
        // This is the standard font color.  It can be copied directly without
        // any need for special processing.
        _needs_image_processing = false;
      }

    } else {
      // This won't be a very interesting font.
      _tex_format = Texture::F_luminance;
    }
  }
}

/**
 * Slots a space in the texture map for the new character and renders the
 * glyph, returning the newly-created TextGlyph object, or NULL if the glyph
 * cannot be created for some reason.
 */
CPT(TextGlyph) DynamicTextFont::
make_glyph(int character, FT_Face face, int glyph_index) {
  if (!load_glyph(face, glyph_index, false)) {
    return nullptr;
  }

  FT_GlyphSlot slot = face->glyph;
  FT_Bitmap &bitmap = slot->bitmap;

  if ((bitmap.width == 0 || bitmap.rows == 0) && (glyph_index == 0)) {
    // Here's a special case: a glyph_index of 0 means an invalid glyph.  Some
    // fonts define a symbol to represent an invalid glyph, but if that symbol
    // is the empty bitmap, we return NULL, and use Panda's invalid glyph in
    // its place.  We do this to guarantee that every invalid glyph is visible
    // as *something*.
    return nullptr;
  }

  PN_stdfloat advance = slot->advance.x / 64.0;
  advance /= _font_pixels_per_unit;

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

    // Ask FreeType to extract the contours out of the outline description.
    decompose_outline(slot->outline);

    PT(TextGlyph) glyph =
      new TextGlyph(character, advance);
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
    case RM_distance_field:
    default:
      break;
    }
  }

  PN_stdfloat tex_x_size, tex_y_size, tex_x_orig, tex_y_orig;
  FT_BBox bounds;
  TransparencyAttrib::Mode alpha_mode;

  if (_render_mode == RM_texture) {
    // Render the glyph if necessary.
    if (slot->format != ft_glyph_format_bitmap) {
      FT_Render_Glyph(slot, ft_render_mode_normal);
    }

    tex_x_size = bitmap.width;
    tex_y_size = bitmap.rows;
    tex_x_orig = slot->bitmap_left;
    tex_y_orig = slot->bitmap_top;
    alpha_mode = TransparencyAttrib::M_alpha;

  } else {
    // Calculate suitable texture dimensions for the signed distance field.
    // This is the same calculation that Freetype uses in its bitmap renderer.
    FT_Outline_Get_CBox(&slot->outline, &bounds);

    bounds.xMin = bounds.xMin & ~63;
    bounds.yMin = bounds.yMin & ~63;
    bounds.xMax = (bounds.xMax + 63) & ~63;
    bounds.yMax = (bounds.yMax + 63) & ~63;

    tex_x_size = (bounds.xMax - bounds.xMin) >> 6;
    tex_y_size = (bounds.yMax - bounds.yMin) >> 6;
    tex_x_orig = (bounds.xMin >> 6);
    tex_y_orig = (bounds.yMax >> 6);
    alpha_mode = TransparencyAttrib::M_binary;
  }

  if (tex_x_size == 0 || tex_y_size == 0) {
    // If we got an empty bitmap, it's a special case.

    PT(TextGlyph) glyph =
      new DynamicTextGlyph(character, advance);
    _empty_glyphs.push_back(glyph);
    return glyph;

  } else {
    DynamicTextGlyph *glyph;

    int outline = 0;

    if (_render_mode == RM_distance_field) {
      tex_x_size /= _scale_factor;
      tex_y_size /= _scale_factor;
      int int_x_size = (int)ceil(tex_x_size);
      int int_y_size = (int)ceil(tex_y_size);

      outline = 4;
      int_x_size += outline * 2;
      int_y_size += outline * 2;
      tex_x_size += outline * 2;
      tex_y_size += outline * 2;

      PNMImage image(int_x_size, int_y_size, PNMImage::CT_grayscale);
      render_distance_field(image, outline, bounds.xMin, bounds.yMin);

      glyph = slot_glyph(character, int_x_size, int_y_size, advance);
      if (!_needs_image_processing) {
        copy_pnmimage_to_texture(image, glyph);
      } else {
        blend_pnmimage_to_texture(image, glyph, _fg);
      }

    } else if (_tex_pixels_per_unit == _font_pixels_per_unit &&
               !_needs_image_processing) {
      // If the bitmap produced from the font doesn't require scaling or any
      // other processing before it goes to the texture, we can just copy it
      // directly into the texture.
      glyph = slot_glyph(character, bitmap.width, bitmap.rows, advance);
      copy_bitmap_to_texture(bitmap, glyph);

    } else {
      // Otherwise, we need to copy to a PNMImage first, so we can scale it
      // andor process it; and then copy it to the texture from there.
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
      glyph = slot_glyph(character, int_x_size, int_y_size, advance);

      if (outline != 0) {
        // Pad the glyph image to make room for the outline.
        PNMImage padded(int_x_size, int_y_size, PNMImage::CT_grayscale);
        padded.copy_sub_image(reduced, outline, outline);
        copy_pnmimage_to_texture(padded, glyph);

      } else {
        copy_pnmimage_to_texture(reduced, glyph);
      }
    }

    DynamicTextPage *page = glyph->get_page();
    if (page != nullptr) {
      int bitmap_top = (int)floor(tex_y_orig + outline * _scale_factor + 0.5f);
      int bitmap_left = (int)floor(tex_x_orig - outline * _scale_factor + 0.5f);

      tex_x_size += glyph->_margin * 2;
      tex_y_size += glyph->_margin * 2;

      // Determine the corners of the rectangle in geometric units.
      PN_stdfloat tex_poly_margin = _poly_margin / _tex_pixels_per_unit;
      PN_stdfloat origin_y = bitmap_top / _font_pixels_per_unit;
      PN_stdfloat origin_x = bitmap_left / _font_pixels_per_unit;

      LVecBase4 dimensions(
        origin_x - tex_poly_margin,
        origin_y - tex_y_size / _tex_pixels_per_unit - tex_poly_margin,
        origin_x + tex_x_size / _tex_pixels_per_unit + tex_poly_margin,
        origin_y + tex_poly_margin);

      // And the corresponding corners in UV units.  We add 0.5f to center the
      // UV in the middle of its texel, to minimize roundoff errors when we
      // are close to 1-to-1 pixel size.
      LVecBase2i page_size = page->get_size();
      LVecBase4 texcoords(
        ((PN_stdfloat)(glyph->_x - _poly_margin) + 0.5f) / page_size[0],
        1.0f - ((PN_stdfloat)(glyph->_y + _poly_margin + tex_y_size) + 0.5f) / page_size[1],
        ((PN_stdfloat)(glyph->_x + _poly_margin + tex_x_size) + 0.5f) / page_size[0],
        1.0f - ((PN_stdfloat)(glyph->_y - _poly_margin) + 0.5f) / page_size[1]);

      CPT(RenderState) state;
      state = RenderState::make(TextureAttrib::make(page),
                                TransparencyAttrib::make(alpha_mode));
      state = state->add_attrib(ColorAttrib::make_flat(LColor(1.0f, 1.0f, 1.0f, 1.0f)), -1);

      glyph->set_quad(dimensions, texcoords, state);
    }

    return glyph;
  }
}

/**
 * Copies a bitmap as rendered by FreeType directly into the texture memory
 * image for the indicated glyph, without any scaling of pixels.
 */
void DynamicTextFont::
copy_bitmap_to_texture(const FT_Bitmap &bitmap, DynamicTextGlyph *glyph) {
  if (bitmap.pixel_mode == ft_pixel_mode_grays && bitmap.num_grays == 256) {
    // This is the easy case: we can memcpy the rendered glyph directly into
    // our texture image, one row at a time.
    unsigned char *buffer_row = bitmap.buffer;
    for (int yi = 0; yi < (int)bitmap.rows; yi++) {

      unsigned char *texture_row = glyph->get_row(yi);
      nassertv(texture_row != nullptr);
      memcpy(texture_row, buffer_row, bitmap.width);
      buffer_row += bitmap.pitch;
    }

  } else if (bitmap.pixel_mode == ft_pixel_mode_mono) {
    // This is a little bit more work: we have to expand the one-bit-per-pixel
    // bitmap into a one-byte-per-pixel texture.
    unsigned char *buffer_row = bitmap.buffer;
    for (int yi = 0; yi < (int)bitmap.rows; yi++) {
      unsigned char *texture_row = glyph->get_row(yi);
      nassertv(texture_row != nullptr);

      int bit = 0x80;
      unsigned char *b = buffer_row;
      for (int xi = 0; xi < (int)bitmap.width; xi++) {
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
    // Here we must expand a grayscale pixmap with n levels of gray into our
    // 256-level texture.
    unsigned char *buffer_row = bitmap.buffer;
    for (int yi = 0; yi < (int)bitmap.rows; yi++) {
      unsigned char *texture_row = glyph->get_row(yi);
      nassertv(texture_row != nullptr);
      for (int xi = 0; xi < (int)bitmap.width; xi++) {
        texture_row[xi] = (int)(buffer_row[xi] * 255) / (bitmap.num_grays - 1);
      }
      buffer_row += bitmap.pitch;
    }

  } else {
    text_cat.error()
      << "Unexpected pixel mode in bitmap: " << (int)bitmap.pixel_mode << "\n";
  }
}

/**
 * Copies a bitmap stored in a PNMImage into the texture memory image for the
 * indicated glyph.
 */
void DynamicTextFont::
copy_pnmimage_to_texture(const PNMImage &image, DynamicTextGlyph *glyph) {
  if (!_needs_image_processing) {
    // Copy the image directly into the alpha component of the texture.
    nassertv(glyph->_page->get_num_components() == 1);
    for (int yi = 0; yi < image.get_y_size(); yi++) {
      unsigned char *texture_row = glyph->get_row(yi);
      nassertv(texture_row != nullptr);
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
      // _outline_feather first to make the range more visually linear (this
      // approximately compensates for the Gaussian falloff of the feathered
      // edge).
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

    // Colorize the image as we copy it in.  This assumes the previous color
    // at this part of the texture was already initialized to the background
    // color.
    blend_pnmimage_to_texture(image, glyph, _fg);
  }
}

/**
 * Blends the PNMImage into the appropriate part of the texture, where 0.0 in
 * the image indicates the color remains the same, and 1.0 indicates the color
 * is assigned the indicated foreground color.
 */
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
      nassertv(texture_row != nullptr);
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
      nassertv(texture_row != nullptr);
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
      nassertv(texture_row != nullptr);
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
      nassertv(texture_row != nullptr);
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

/**
 * Chooses a page that will have room for a glyph of the indicated size (after
 * expanding the indicated size by the current margin).  Returns the newly-
 * allocated glyph on the chosen page; the glyph has not been filled in yet
 * except with its size.
 */
DynamicTextGlyph *DynamicTextFont::
slot_glyph(int character, int x_size, int y_size, PN_stdfloat advance) {
  // Increase the indicated size by the current margin.
  x_size += _texture_margin * 2;
  y_size += _texture_margin * 2;

  if (!_pages.empty()) {
    // Start searching on the preferred page.  That way, we'll fill up the
    // preferred page first, and we can gradually rotate this page around; it
    // keeps us from spending too much time checking already-filled pages for
    // space.
    _preferred_page = _preferred_page % _pages.size();
    int pi = _preferred_page;

    do {
      DynamicTextPage *page = _pages[pi];
      DynamicTextGlyph *glyph = page->slot_glyph(character, x_size, y_size, _texture_margin, advance);
      if (glyph != nullptr) {
        // Once we found a page to hold the glyph, that becomes our new
        // preferred page.
        _preferred_page = pi;
        return glyph;
      }

      if (page->is_empty()) {
        // If we couldn't even put it on an empty page, we're screwed.
        text_cat.error()
          << "Glyph of size " << x_size << " by " << y_size
          << " pixels won't fit on an empty page.\n";
        return nullptr;
      }

      pi = (pi + 1) % _pages.size();
    } while (pi != _preferred_page);
  }

  // All pages are filled.  Can we free up space by removing some old glyphs?
  if (garbage_collect() != 0) {
    // Yes, we just freed up some space.  Try once more, recursively.
    return slot_glyph(character, x_size, y_size, advance);

  } else {
    // No good; all recorded glyphs are actually in use.  We need to make a
    // new page.
    _preferred_page = _pages.size();
    PT(DynamicTextPage) page = new DynamicTextPage(this, _preferred_page);
    _pages.push_back(page);
    return page->slot_glyph(character, x_size, y_size, _texture_margin, advance);
  }
}

/**
 * Converts from the _contours list to an actual glyph geometry, as a
 * wireframe render.
 */
void DynamicTextFont::
render_wireframe_contours(TextGlyph *glyph) {
  PT(GeomVertexData) vdata = new GeomVertexData
    (std::string(), GeomVertexFormat::get_v3(),
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

/**
 * Converts from the _contours list to an actual glyph geometry, as a polygon
 * render.
 */
void DynamicTextFont::
render_polygon_contours(TextGlyph *glyph, bool face, bool extrude) {
  PT(GeomVertexData) vdata = new GeomVertexData
    (std::string(), GeomVertexFormat::get_v3n3(),
     Geom::UH_static);
  GeomVertexWriter vertex(vdata, InternalName::get_vertex());
  GeomVertexWriter normal(vdata, InternalName::get_normal());

  PT(GeomTriangles) tris = new GeomTriangles(Geom::UH_static);
  Triangulator t;

  Contours::iterator ci;

  if (face) {
    // First, build up the list of vertices for the face, and determine which
    // contours are solid and which are holes.
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
    // If we're generating extruded geometry (polygons along the edges, down
    // the y axis), generate them now.  These are pretty easy, but we need to
    // create more vertices--they don't share the same normals.
    for (ci = _contours.begin(); ci != _contours.end(); ++ci) {
      const Contour &contour = (*ci);

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
          // If the out tangent is different from the in tangent, we need to
          // store new vertices for the next quad.
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
  // glyph->set_geom(vdata, tris, RenderState::make(RenderModeAttrib::make(Ren
  // derModeAttrib::M_wireframe))); glyph->set_geom(vdata, tris,
  // RenderState::make(AntialiasAttrib::make(AntialiasAttrib::M_auto)));
  _contours.clear();
}

#endif  // HAVE_FREETYPE
