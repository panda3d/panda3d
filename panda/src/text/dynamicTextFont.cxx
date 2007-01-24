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

#undef interface  // I don't know where this symbol is defined, but it interferes with FreeType.
#include FT_OUTLINE_H 
#include FT_STROKER_H
#include FT_BBOX_H
#ifdef FT_BITMAP_H
#include FT_BITMAP_H
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
    DynamicTextGlyph *dynamic_glyph = make_glyph(character, glyph_index);
    _cache.insert(Cache::value_type(glyph_index, dynamic_glyph));
    glyph = dynamic_glyph;
  }

  return (glyph_index != 0 && glyph != (DynamicTextGlyph *)NULL);
}


////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::string_render_mode
//       Access: Public
//  Description: Returns the RenderMode value associated with the given
//               string representation, or RM_invalid if the string
//               does not match any known RenderMode value.
////////////////////////////////////////////////////////////////////
DynamicTextFont::RenderMode DynamicTextFont::
string_render_mode(const string &string) {
  if (cmp_nocase_uh(string, "texture") == 0) {
    return RM_texture;
  } else if (cmp_nocase_uh(string, "wireframe") == 0) {
    return RM_wireframe;
  } else if (cmp_nocase_uh(string, "polygon") == 0) {
    return RM_polygon;
  } else if (cmp_nocase_uh(string, "extruded") == 0) {
    return RM_extruded;
  } else if (cmp_nocase_uh(string, "solid") == 0) {
    return RM_solid;
  } else {
    return RM_invalid;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::string_winding_order
//       Access: Public
//  Description: Returns the WindingOrder value associated with the given
//               string representation, or WO_invalid if the string
//               does not match any known WindingOrder value.
////////////////////////////////////////////////////////////////////
DynamicTextFont::WindingOrder DynamicTextFont::
string_winding_order(const string &string) {
  if (cmp_nocase_uh(string, "default") == 0) {
    return WO_default;
  } else if (cmp_nocase_uh(string, "left") == 0) {
    return WO_left;
  } else if (cmp_nocase_uh(string, "right") == 0) {
    return WO_right;
  } else {
    return WO_invalid;
  }
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
//     Function: DynamicTextFont::make_glyph
//       Access: Private
//  Description: Slots a space in the texture map for the new
//               character and renders the glyph, returning the
//               newly-created TextGlyph object, or NULL if the
//               glyph cannot be created for some reason.
////////////////////////////////////////////////////////////////////
DynamicTextGlyph *DynamicTextFont::
make_glyph(int character, int glyph_index) {
  if (!load_glyph(glyph_index, false)) {
    return (DynamicTextGlyph *)NULL;
  }

  FT_GlyphSlot slot = _face->glyph;
  FT_Bitmap &bitmap = slot->bitmap;

  float advance = slot->advance.x / 64.0;

  if (_render_mode != RM_texture && 
      slot->format == ft_glyph_format_outline) {
    // Re-stroke the glyph to make it an outline glyph.
    /*
    FT_Stroker stroker;
    FT_Stroker_New(_face->memory, &stroker);
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
      if (FT_Outline_Get_Orientation(&slot->outline) == FT_ORIENTATION_FILL_RIGHT) {
        wo = WO_right;
      } else {
        wo = WO_left;
      }
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
      render_polygon_contours(glyph);
      return glyph;

    default:
      break;
    }
  }

  // Render the glyph if necessary.
  if (slot->format != ft_glyph_format_bitmap) {
    FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL);
  }

  if (bitmap.width == 0 || bitmap.rows == 0) {
    // If we got an empty bitmap, it's a special case.
    PT(DynamicTextGlyph) glyph = 
      new DynamicTextGlyph(character, advance / _font_pixels_per_unit);
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
      glyph = slot_glyph(character, bitmap.width, bitmap.rows);
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
      glyph = slot_glyph(character, int_x_size, int_y_size);
      
      PNMImage image(bmp_x_size, bmp_y_size, PNMImage::CT_grayscale);
      copy_bitmap_to_pnmimage(bitmap, image);

      PNMImage reduced(int_x_size, int_y_size, PNMImage::CT_grayscale);
      reduced.quick_filter_from(image);
      copy_pnmimage_to_texture(reduced, glyph);
    }
      
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
      const LPoint2f &p = (*pi);
      vertex.add_data3f(p[0], 0.0f, p[1]);
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
render_polygon_contours(DynamicTextGlyph *glyph) {
  PT(GeomVertexData) vdata = new GeomVertexData
    (string(), GeomVertexFormat::get_v3(),
     Geom::UH_static);
  GeomVertexWriter vertex(vdata, InternalName::get_vertex());

  PT(GeomTriangles) tris = new GeomTriangles(Geom::UH_static);
  Triangulator t;

  // First, build up the list of vertices, and determine which
  // contours are solid and which are holes.
  Contours::iterator ci;
  for (ci = _contours.begin(); ci != _contours.end(); ++ci) {
    Contour &contour = (*ci);

    t.clear_polygon();
    contour._start_vertex = t.get_num_vertices();
    for (size_t i = 0; i < contour._points.size() - 1; ++i) {
      const LPoint2f &p = contour._points[i];
      vertex.add_data3f(p[0], 0.0f, p[1]);
      int vi = t.add_vertex(p[0], p[1]);
      t.add_polygon_vertex(vi);
    }

    contour._is_solid = t.is_left_winding();
  }

  // Now go back and generate the actual triangles.
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

  glyph->set_geom(vdata, tris, RenderState::make_empty());
  //  glyph->set_geom(vdata, tris, RenderState::make(RenderModeAttrib::make(RenderModeAttrib::M_wireframe)));
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
  float scale = 1.0f / (64.0f * self->_font_pixels_per_unit);
  LPoint2f p = LPoint2f(to->x, to->y) * scale;

  self->_contours.push_back(Contour());
  self->_contours.back()._points.push_back(p);
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
  float scale = 1.0f / (64.0f * self->_font_pixels_per_unit);
  LPoint2f p = LPoint2f(to->x, to->y) * scale;

  self->_contours.back()._points.push_back(p);
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
  const LPoint2f &q = self->_contours.back()._points.back();

  // Convert from 26.6 pixel units to Panda units.
  float scale = 1.0f / (64.0f * self->_font_pixels_per_unit);

  LPoint2f c = LPoint2f(control->x, control->y) * scale;
  LPoint2f p = LPoint2f(to->x, to->y) * scale;

  // The NurbsCurveEvaluator will evaluate the Bezier segment for us.
  NurbsCurveEvaluator nce;
  nce.local_object();
  nce.set_order(3);
  nce.reset(3);
  nce.set_vertex(0, LVecBase3f(q[0], q[1], 0.0f));
  nce.set_vertex(1, LVecBase3f(c[0], c[1], 0.0f));
  nce.set_vertex(2, LVecBase3f(p[0], p[1], 0.0f));

  PT(NurbsCurveResult) ncr = nce.evaluate();

  // Sample it down so that the lines approximate the curve to within
  // a "pixel."
  ncr->adaptive_sample(1.0f / self->_font_pixels_per_unit);

  int num_samples = ncr->get_num_samples();
  for (int i = 1; i < num_samples; ++i) {
    const LPoint3f &p = ncr->get_sample_point(i);
    self->_contours.back()._points.push_back(LPoint2f(p[0], p[1]));
  }
  return 0;
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
  const LPoint2f &q = self->_contours.back()._points.back();

  // Convert from 26.6 pixel units to Panda units.
  float scale = 1.0f / (64.0f * self->_font_pixels_per_unit);

  LPoint2f c1 = LPoint2f(control1->x, control1->y) * scale;
  LPoint2f c2 = LPoint2f(control2->x, control2->y) * scale;
  LPoint2f p = LPoint2f(to->x, to->y) * scale;

  // The NurbsCurveEvaluator will evaluate the Bezier segment for us.
  NurbsCurveEvaluator nce;
  nce.local_object();
  nce.set_order(4);
  nce.reset(4);
  nce.set_vertex(0, LVecBase3f(q[0], q[1], 0.0f));
  nce.set_vertex(1, LVecBase3f(c1[0], c1[1], 0.0f));
  nce.set_vertex(2, LVecBase3f(c2[0], c2[1], 0.0f));
  nce.set_vertex(3, LVecBase3f(p[0], p[1], 0.0f));

  PT(NurbsCurveResult) ncr = nce.evaluate();

  // Sample it down so that the lines approximate the curve to within
  // a "pixel."
  ncr->adaptive_sample(1.0f / self->_font_pixels_per_unit);

  int num_samples = ncr->get_num_samples();
  for (int i = 1; i < num_samples; ++i) {
    const LPoint3f &p = ncr->get_sample_point(i);
    self->_contours.back()._points.push_back(LPoint2f(p[0], p[1]));
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::RenderMode output operator
//  Description:
////////////////////////////////////////////////////////////////////
ostream &
operator << (ostream &out, DynamicTextFont::RenderMode rm) {
  switch (rm) {
  case DynamicTextFont::RM_texture:
    return out << "texture";
  case DynamicTextFont::RM_wireframe:
    return out << "wireframe";
  case DynamicTextFont::RM_polygon:
    return out << "polygon";
  case DynamicTextFont::RM_extruded:
    return out << "extruded";
  case DynamicTextFont::RM_solid:
    return out << "solid";

  case DynamicTextFont::RM_invalid:
    return out << "invalid";
  }

  return out << "(**invalid DynamicTextFont::RenderMode(" << (int)rm << ")**)";
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::RenderMode input operator
//  Description:
////////////////////////////////////////////////////////////////////
istream &
operator >> (istream &in, DynamicTextFont::RenderMode &rm) {
  string word;
  in >> word;

  rm = DynamicTextFont::string_render_mode(word);
  return in;
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::WindingOrder output operator
//  Description:
////////////////////////////////////////////////////////////////////
ostream &
operator << (ostream &out, DynamicTextFont::WindingOrder wo) {
  switch (wo) {
  case DynamicTextFont::WO_default:
    return out << "default";
  case DynamicTextFont::WO_left:
    return out << "left";
  case DynamicTextFont::WO_right:
    return out << "right";

  case DynamicTextFont::WO_invalid:
    return out << "invalid";
  }

  return out << "(**invalid DynamicTextFont::WindingOrder(" << (int)wo << ")**)";
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::WindingOrder input operator
//  Description:
////////////////////////////////////////////////////////////////////
istream &
operator >> (istream &in, DynamicTextFont::WindingOrder &wo) {
  string word;
  in >> word;

  wo = DynamicTextFont::string_winding_order(word);
  return in;
}

#endif  // HAVE_FREETYPE
