/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file freetypeFont.h
 * @author drose
 * @date 2003-09-07
 */

#ifndef FREETYPEFONT_H
#define FREETYPEFONT_H

#include "pandabase.h"

#ifdef HAVE_FREETYPE

#include "config_pnmtext.h"
#include "filename.h"
#include "pvector.h"
#include "pmap.h"
#include "pnmImage.h"
#include "namable.h"
#include "freetypeFace.h"

#include <ft2build.h>
#include FT_FREETYPE_H

class NurbsCurveResult;

/**
 * This is a common base class for both DynamicTextFont and PNMTextMaker.
 * Both of these are utility classes that use the FreeType library to generate
 * glyphs from fonts; this class abstracts out that common wrapper around
 * FreeType.
 */
class EXPCL_PANDA_PNMTEXT FreetypeFont : public Namable {
protected:
  FreetypeFont();
  FreetypeFont(const FreetypeFont &copy);

  bool load_font(const Filename &font_filename, int face_index);
  bool load_font(const char *font_data, int data_length, int face_index);
  void unload_font();

PUBLISHED:
  INLINE ~FreetypeFont();

  enum WindingOrder {
    WO_default,
    WO_left,
    WO_right,

    WO_invalid,
  };

  INLINE bool set_point_size(PN_stdfloat point_size);
  INLINE PN_stdfloat get_point_size() const;

  INLINE bool set_pixels_per_unit(PN_stdfloat pixels_per_unit);
  INLINE PN_stdfloat get_pixels_per_unit() const;

  INLINE bool set_pixel_size(PN_stdfloat pixel_size);
  INLINE PN_stdfloat get_pixel_size() const;

  INLINE bool set_scale_factor(PN_stdfloat scale_factor);
  INLINE PN_stdfloat get_scale_factor() const;

  INLINE void set_native_antialias(bool native_antialias);
  INLINE bool get_native_antialias() const;

  INLINE int get_font_pixel_size() const;

  INLINE PN_stdfloat get_line_height() const;
  INLINE PN_stdfloat get_space_advance() const;

  INLINE static PN_stdfloat get_points_per_unit();
  INLINE static PN_stdfloat get_points_per_inch();

  INLINE void set_winding_order(WindingOrder winding_order);
  INLINE WindingOrder get_winding_order() const;
  MAKE_PROPERTY(winding_order, get_winding_order, set_winding_order);

public:
  static WindingOrder string_winding_order(const std::string &string);

protected:
  INLINE FT_Face acquire_face() const;
  INLINE void release_face(FT_Face face) const;

  bool load_glyph(FT_Face face, int glyph_index, bool prerender = true);
  void copy_bitmap_to_pnmimage(const FT_Bitmap &bitmap, PNMImage &image);
  void render_distance_field(PNMImage &image, int radius, int min_x, int min_y);

  void decompose_outline(FT_Outline &outline);

private:
  bool reset_scale();

  static int outline_move_to(const FT_Vector *to, void *user);
  static int outline_line_to(const FT_Vector *to, void *user);
  static int outline_conic_to(const FT_Vector *control,
                              const FT_Vector *to, void *user);
  static int outline_cubic_to(const FT_Vector *control1,
                              const FT_Vector *control2,
                              const FT_Vector *to, void *user);
  int outline_nurbs(NurbsCurveResult *ncr);

protected:
  PN_stdfloat _point_size;
  PN_stdfloat _requested_pixels_per_unit;
  PN_stdfloat _tex_pixels_per_unit;
  PN_stdfloat _requested_scale_factor;
  PN_stdfloat _scale_factor;
  bool _native_antialias;
  PN_stdfloat _font_pixels_per_unit;
  WindingOrder _winding_order;

  int _font_pixel_size;
  PN_stdfloat _line_height;
  PN_stdfloat _space_advance;

  PT(FreetypeFace) _face;
  int _char_size;
  int _dpi;
  int _pixel_width;
  int _pixel_height;

  class ContourPoint {
  public:
    INLINE ContourPoint(const LPoint2 &p, const LVector2 &in,
                        const LVector2 &out);
    INLINE ContourPoint(PN_stdfloat px, PN_stdfloat py, PN_stdfloat tx, PN_stdfloat ty);
    INLINE void connect_to(const LVector2 &out);
    LPoint2 _p;
    LVector2 _in, _out;  // tangents into and out of the vertex.

    // Circular arc approximation of the curve from previous point.  If radius
    // is 0, this is a straight line.
    LPoint2 _center;
    PN_stdfloat _radius;
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
  LPoint2 _q;  // The "current point".

protected:
  static const PN_stdfloat _points_per_unit;
  static const PN_stdfloat _points_per_inch;
};

#include "freetypeFont.I"

EXPCL_PANDA_PNMTEXT std::ostream &operator << (std::ostream &out, FreetypeFont::WindingOrder wo);
EXPCL_PANDA_PNMTEXT std::istream &operator >> (std::istream &in, FreetypeFont::WindingOrder &wo);

#endif  // HAVE_FREETYPE

#endif
