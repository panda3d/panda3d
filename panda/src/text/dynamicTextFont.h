// Filename: dynamicTextFont.h
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

////////////////////////////////////////////////////////////////////
//       Class : DynamicTextFont
// Description : A DynamicTextFont is a special TextFont object that
//               rasterizes its glyphs from a standard font file
//               (e.g. a TTF file) on the fly.  It requires the
//               FreeType 2.0 library (or any higher,
//               backward-compatible version).
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_TEXT DynamicTextFont : public TextFont, public FreetypeFont {
PUBLISHED:
  DynamicTextFont(const Filename &font_filename, int face_index = 0);
  DynamicTextFont(const char *font_data, int data_length, int face_index);
  virtual ~DynamicTextFont();

  INLINE const string &get_name() const;

  INLINE bool set_point_size(float point_size);
  INLINE float get_point_size() const;

  INLINE bool set_pixels_per_unit(float pixels_per_unit);
  INLINE float get_pixels_per_unit() const;

  INLINE bool set_scale_factor(float scale_factor);
  INLINE float get_scale_factor() const;

  INLINE void set_native_antialias(bool native_antialias);
  INLINE bool get_native_antialias() const;

  INLINE int get_font_pixel_size() const;

  INLINE float get_line_height() const;
  INLINE float get_space_advance() const;

  INLINE void set_texture_margin(int texture_margin);
  INLINE int get_texture_margin() const;
  INLINE void set_poly_margin(float poly_margin);
  INLINE float get_poly_margin() const;

  INLINE void set_page_size(int x_size, int y_size);
  INLINE int get_page_x_size() const;
  INLINE int get_page_y_size() const;

  INLINE void set_minfilter(Texture::FilterType filter);
  INLINE Texture::FilterType get_minfilter() const;
  INLINE void set_magfilter(Texture::FilterType filter);
  INLINE Texture::FilterType get_magfilter() const;
  INLINE void set_anisotropic_degree(int anisotropic_degree);
  INLINE int get_anisotropic_degree() const;

  INLINE void set_render_mode(RenderMode render_mode);
  INLINE RenderMode get_render_mode() const;
  INLINE void set_winding_order(WindingOrder winding_order);
  INLINE WindingOrder get_winding_order() const;

  INLINE void set_fg(const Colorf &fg);
  INLINE const Colorf &get_fg() const;
  INLINE void set_bg(const Colorf &bg);
  INLINE const Colorf &get_bg() const;
  INLINE void set_outline(const Colorf &outline_color, float outline_width,
                          float outline_feather);
  INLINE const Colorf &get_outline_color() const;
  INLINE float get_outline_width() const;
  INLINE float get_outline_feather() const;
  INLINE Texture::Format get_tex_format() const;

  int get_num_pages() const;
  DynamicTextPage *get_page(int n) const;
  MAKE_SEQ(get_pages, get_num_pages, get_page);

  int garbage_collect();
  void clear();

  virtual void write(ostream &out, int indent_level) const;

public:
  virtual bool get_glyph(int character, const TextGlyph *&glyph);

private:
  void initialize();
  void update_filters();
  void determine_tex_format();
  DynamicTextGlyph *make_glyph(int character, int glyph_index);
  void copy_bitmap_to_texture(const FT_Bitmap &bitmap, DynamicTextGlyph *glyph);
  void copy_pnmimage_to_texture(const PNMImage &image, DynamicTextGlyph *glyph);
  void blend_pnmimage_to_texture(const PNMImage &image, DynamicTextGlyph *glyph,
                                 const Colorf &fg);
  DynamicTextGlyph *slot_glyph(int character, int x_size, int y_size);

  void render_wireframe_contours(DynamicTextGlyph *glyph);
  void render_polygon_contours(DynamicTextGlyph *glyph, bool face, bool extrude);

  static int outline_move_to(const FT_Vector *to, void *user);
  static int outline_line_to(const FT_Vector *to, void *user);
  static int outline_conic_to(const FT_Vector *control,
                              const FT_Vector *to, void *user);
  static int outline_cubic_to(const FT_Vector *control1, 
                              const FT_Vector *control2, 
                              const FT_Vector *to, void *user);
  int outline_nurbs(NurbsCurveResult *ncr);

  int _texture_margin;
  float _poly_margin;
  int _page_x_size, _page_y_size;

  Texture::FilterType _minfilter;
  Texture::FilterType _magfilter;
  int _anisotropic_degree;

  RenderMode _render_mode;
  WindingOrder _winding_order;

  Colorf _fg, _bg, _outline_color;
  float _outline_width;
  float _outline_feather;
  bool _has_outline;
  Texture::Format _tex_format;
  bool _needs_image_processing;

  typedef pvector< PT(DynamicTextPage) > Pages;
  Pages _pages;
  int _preferred_page;

  // This doesn't need to be a reference-counting pointer, because the
  // reference to each glyph is kept by the DynamicTextPage object.
  typedef pmap<int, DynamicTextGlyph *> Cache;
  Cache _cache;

  // This is a list of the glyphs that do not have any printable
  // properties (e.g. space), but still have an advance measure.  We
  // store them here to keep their reference counts; they also appear
  // in the above table.
  typedef pvector< PT(DynamicTextGlyph) > EmptyGlyphs;
  EmptyGlyphs _empty_glyphs;

  class ContourPoint {
  public:
    INLINE ContourPoint(const LPoint2f &p, const LVector2f &in, 
                        const LVector2f &out);
    INLINE ContourPoint(float px, float py, float tx, float ty);
    INLINE void connect_to(const LVector2f &out);
    LPoint2f _p;
    LVector2f _in, _out;  // tangents into and out of the vertex.
  };
  typedef pvector<ContourPoint> Points;

  class Contour {
  public:
    Points _points;
    bool _is_solid;
    int _start_vertex;
  };

  typedef pvector<Contour> Contours;
  Contours _contours;
  LPoint2f _q;  // The "current point".

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

INLINE ostream &operator << (ostream &out, const DynamicTextFont &dtf);

#include "dynamicTextFont.I"

#endif  // HAVE_FREETYPE

#endif
