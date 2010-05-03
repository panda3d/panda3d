// Filename: freetypeFont.h
// Created by:  drose (07Sep03)
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

////////////////////////////////////////////////////////////////////
//       Class : FreetypeFont
// Description : This is a common base class for both DynamicTextFont
//               and PNMTextMaker.  Both of these are utility classes
//               that use the FreeType library to generate glyphs from
//               fonts; this class abstracts out that common wrapper
//               around FreeType.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PNMTEXT FreetypeFont : public Namable {
protected:
  FreetypeFont();
  FreetypeFont(const FreetypeFont &copy);

  bool load_font(const Filename &font_filename, int face_index);
  bool load_font(const char *font_data, int data_length, int face_index);
  void unload_font();

PUBLISHED:
  INLINE ~FreetypeFont();

  INLINE bool set_point_size(float point_size);
  INLINE float get_point_size() const;

  INLINE bool set_pixels_per_unit(float pixels_per_unit);
  INLINE float get_pixels_per_unit() const;

  INLINE bool set_pixel_size(float pixel_size);
  INLINE float get_pixel_size() const;

  INLINE bool set_scale_factor(float scale_factor);
  INLINE float get_scale_factor() const;

  INLINE void set_native_antialias(bool native_antialias);
  INLINE bool get_native_antialias() const;

  INLINE int get_font_pixel_size() const;

  INLINE float get_line_height() const;
  INLINE float get_space_advance() const;

  INLINE static float get_points_per_unit();
  INLINE static float get_points_per_inch();

protected:
  INLINE FT_Face acquire_face() const;
  INLINE void release_face(FT_Face face) const;

  bool load_glyph(FT_Face face, int glyph_index, bool prerender = true);
  void copy_bitmap_to_pnmimage(const FT_Bitmap &bitmap, PNMImage &image);

private:
  bool reset_scale();

protected:
  float _point_size;
  float _requested_pixels_per_unit;
  float _tex_pixels_per_unit;
  float _requested_scale_factor;
  float _scale_factor;
  bool _native_antialias;
  float _font_pixels_per_unit;

  int _font_pixel_size;
  float _line_height;
  float _space_advance;

  PT(FreetypeFace) _face;
  int _char_size;
  int _dpi;
  int _pixel_width;
  int _pixel_height;

protected:
  static const float _points_per_unit;
  static const float _points_per_inch;
};

#include "freetypeFont.I"

#endif  // HAVE_FREETYPE

#endif
