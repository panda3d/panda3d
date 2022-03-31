/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dynamicTextFont.h
 * @author drose
 * @date 2002-02-08
 */

#ifndef DYNAMICTEXTFONT_H
#define DYNAMICTEXTFONT_H

#include "pandabase.h"

#ifdef HAVE_FREETYPE

#include "textFont.h"
#include "freetypeFont.h"
#include "dynamicTextGlyph.h"
#include "dynamicTextPage.h"
#include "filename.h"
#include "pvector.h"
#include "pmap.h"

#include <ft2build.h>
#include FT_FREETYPE_H

class NurbsCurveResult;

typedef struct hb_font_t hb_font_t;

/**
 * A DynamicTextFont is a special TextFont object that rasterizes its glyphs
 * from a standard font file (e.g.  a TTF file) on the fly.  It requires the
 * FreeType 2.0 library (or any higher, backward-compatible version).
 */
class EXPCL_PANDA_TEXT DynamicTextFont : public TextFont, public FreetypeFont {
PUBLISHED:
  DynamicTextFont(const Filename &font_filename, int face_index = 0);
  DynamicTextFont(const char *font_data, int data_length, int face_index);
  DynamicTextFont(const DynamicTextFont &copy);
  virtual ~DynamicTextFont();

  virtual PT(TextFont) make_copy() const;

  INLINE const std::string &get_name() const;

  INLINE bool set_point_size(PN_stdfloat point_size);
  INLINE PN_stdfloat get_point_size() const;
  MAKE_PROPERTY(point_size, get_point_size, set_point_size);

  INLINE bool set_pixels_per_unit(PN_stdfloat pixels_per_unit);
  INLINE PN_stdfloat get_pixels_per_unit() const;
  MAKE_PROPERTY(pixels_per_unit, get_pixels_per_unit, set_pixels_per_unit);

  INLINE bool set_scale_factor(PN_stdfloat scale_factor);
  INLINE PN_stdfloat get_scale_factor() const;
  MAKE_PROPERTY(scale_factor, get_scale_factor, set_scale_factor);

  INLINE void set_native_antialias(bool native_antialias);
  INLINE bool get_native_antialias() const;
  MAKE_PROPERTY(native_antialias, get_native_antialias, set_native_antialias);

  INLINE int get_font_pixel_size() const;
  MAKE_PROPERTY(font_pixel_size, get_font_pixel_size);

  INLINE PN_stdfloat get_line_height() const;
  INLINE PN_stdfloat get_space_advance() const;

  INLINE void set_texture_margin(int texture_margin);
  INLINE int get_texture_margin() const;
  INLINE void set_poly_margin(PN_stdfloat poly_margin);
  INLINE PN_stdfloat get_poly_margin() const;
  MAKE_PROPERTY(texture_margin, get_texture_margin, set_texture_margin);
  MAKE_PROPERTY(poly_margin, get_poly_margin, set_poly_margin);

  INLINE void set_page_size(const LVecBase2i &page_size);
  INLINE void set_page_size(int x_size, int y_size);
  INLINE const LVecBase2i &get_page_size() const;
  INLINE int get_page_x_size() const;
  INLINE int get_page_y_size() const;
  MAKE_PROPERTY(page_size, get_page_size, set_page_size);

  INLINE void set_minfilter(SamplerState::FilterType filter);
  INLINE SamplerState::FilterType get_minfilter() const;
  INLINE void set_magfilter(SamplerState::FilterType filter);
  INLINE SamplerState::FilterType get_magfilter() const;
  INLINE void set_anisotropic_degree(int anisotropic_degree);
  INLINE int get_anisotropic_degree() const;
  MAKE_PROPERTY(minfilter, get_minfilter, set_minfilter);
  MAKE_PROPERTY(magfilter, get_magfilter, set_magfilter);
  MAKE_PROPERTY(anisotropic_degree, get_anisotropic_degree, set_anisotropic_degree);

  INLINE void set_render_mode(RenderMode render_mode);
  INLINE RenderMode get_render_mode() const;
  MAKE_PROPERTY(render_mode, get_render_mode, set_render_mode);

  INLINE void set_fg(const LColor &fg);
  INLINE const LColor &get_fg() const;
  INLINE void set_bg(const LColor &bg);
  INLINE const LColor &get_bg() const;
  INLINE void set_outline(const LColor &outline_color, PN_stdfloat outline_width,
                          PN_stdfloat outline_feather);
  INLINE const LColor &get_outline_color() const;
  INLINE PN_stdfloat get_outline_width() const;
  INLINE PN_stdfloat get_outline_feather() const;
  INLINE Texture::Format get_tex_format() const;
  MAKE_PROPERTY(fg, get_fg, set_fg);
  MAKE_PROPERTY(bg, get_bg, set_bg);
  MAKE_PROPERTY(tex_format, get_tex_format);

  int get_num_pages() const;
  DynamicTextPage *get_page(int n) const;
  MAKE_SEQ(get_pages, get_num_pages, get_page);
  MAKE_SEQ_PROPERTY(pages, get_num_pages, get_page);

  int garbage_collect();
  void clear();

  virtual void write(std::ostream &out, int indent_level) const;

public:
  virtual bool get_glyph(int character, CPT(TextGlyph) &glyph);
  virtual PN_stdfloat get_kerning(int first, int second) const;

  bool get_glyph_by_index(int character, int glyph_index, CPT(TextGlyph) &glyph);
  hb_font_t *get_hb_font() const;

private:
  void initialize();
  void update_filters();
  void determine_tex_format();
  CPT(TextGlyph) make_glyph(int character, FT_Face face, int glyph_index);
  void copy_bitmap_to_texture(const FT_Bitmap &bitmap, DynamicTextGlyph *glyph);
  void copy_pnmimage_to_texture(const PNMImage &image, DynamicTextGlyph *glyph);
  void blend_pnmimage_to_texture(const PNMImage &image, DynamicTextGlyph *glyph,
                                 const LColor &fg);
  DynamicTextGlyph *slot_glyph(int character, int x_size, int y_size, PN_stdfloat advance);

  void render_wireframe_contours(TextGlyph *glyph);
  void render_polygon_contours(TextGlyph *glyph, bool face, bool extrude);

  int _texture_margin;
  PN_stdfloat _poly_margin;
  LVecBase2i _page_size;

  SamplerState::FilterType _minfilter;
  SamplerState::FilterType _magfilter;
  int _anisotropic_degree;

  RenderMode _render_mode;

  LColor _fg, _bg, _outline_color;
  PN_stdfloat _outline_width;
  PN_stdfloat _outline_feather;
  bool _has_outline;
  Texture::Format _tex_format;
  bool _needs_image_processing;

  typedef pvector< PT(DynamicTextPage) > Pages;
  Pages _pages;
  int _preferred_page;

  // This doesn't need to be a reference-counting pointer, because the
  // reference to each glyph is kept by the DynamicTextPage object.
  typedef pmap<int, const TextGlyph *> Cache;
  Cache _cache;

  // This is a list of the glyphs that do not have any printable properties
  // (e.g.  space), but still have an advance measure.  We store them here to
  // keep their reference counts; they also appear in the above table.
  typedef pvector< PT(TextGlyph) > EmptyGlyphs;
  EmptyGlyphs _empty_glyphs;

  mutable hb_font_t *_hb_font;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TextFont::init_type();
    register_type(_type_handle, "DynamicTextFont",
                  TextFont::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class TextNode;
};

INLINE std::ostream &operator << (std::ostream &out, const DynamicTextFont &dtf);

#include "dynamicTextFont.I"

#endif  // HAVE_FREETYPE

#endif
