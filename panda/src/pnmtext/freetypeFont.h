// Filename: freetypeFont.h
// Created by:  drose (07Sep03)
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
class EXPCL_PANDA FreetypeFont : public Namable {
protected:
  FreetypeFont();

  bool load_font(const Filename &font_filename, int face_index);
  bool load_font(const char *font_data, int data_length, int face_index);
  void unload_font();

public:
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
  bool load_glyph(int glyph_index);
  void copy_bitmap_to_pnmimage(const FT_Bitmap &bitmap, PNMImage &image);

private:
  bool font_loaded();
  bool reset_scale();
  static void initialize_ft_library();

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

  FT_Face _face;

private:
  bool _font_loaded;

  // This string is used to hold the data read from the font file in
  // vfs mode.  Since the FreeType library keeps pointers into this
  // data, we have to keep it around.
  string _raw_font_data;

  static FT_Library _ft_library;
  static bool _ft_initialized;
  static bool _ft_ok;

  static const float _points_per_unit;
  static const float _points_per_inch;
};

#include "freetypeFont.I"

#endif  // HAVE_FREETYPE

#endif
