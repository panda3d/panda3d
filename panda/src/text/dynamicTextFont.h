// Filename: dynamicTextFont.h
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
class EXPCL_PANDA DynamicTextFont : public TextFont, public FreetypeFont {
PUBLISHED:
  enum RenderMode {
    // Each glyph is a single textured rectangle
    RM_texture,

    // Each glyph is a lot of line segments
    RM_wireframe,

    // Each glyph is a lot of triangles
    RM_polygon,

    // a 3-D outline, like a cookie cutter
    RM_extruded,

    // combination of RM_extruded and RM_polygon
    RM_solid,

    // Returned by string_render_mode() for an invalid match.
    RM_invalid,
  };

  enum WindingOrder {
    WO_default,
    WO_left,
    WO_right,

    WO_invalid,
  };

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

  int get_num_pages() const;
  DynamicTextPage *get_page(int n) const;

  int garbage_collect();
  void clear();

  virtual void write(ostream &out, int indent_level) const;

public:
  virtual bool get_glyph(int character, const TextGlyph *&glyph);

  static RenderMode string_render_mode(const string &string);
  static WindingOrder string_winding_order(const string &string);

private:
  void initialize();
  void update_filters();
  DynamicTextGlyph *make_glyph(int character, int glyph_index);
  void copy_bitmap_to_texture(const FT_Bitmap &bitmap, DynamicTextGlyph *glyph);
  void copy_pnmimage_to_texture(const PNMImage &image, DynamicTextGlyph *glyph);
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

EXPCL_PANDA ostream &operator << (ostream &out, DynamicTextFont::RenderMode rm);
EXPCL_PANDA istream &operator >> (istream &in, DynamicTextFont::RenderMode &rm);
EXPCL_PANDA ostream &operator << (ostream &out, DynamicTextFont::WindingOrder wo);
EXPCL_PANDA istream &operator >> (istream &in, DynamicTextFont::WindingOrder &wo);

#include "dynamicTextFont.I"

#endif  // HAVE_FREETYPE

#endif
