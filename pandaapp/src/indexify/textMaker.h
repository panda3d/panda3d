// Filename: textMaker.h
// Created by:  drose (03Apr02)
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

#ifndef TEXTMAKER_H
#define TEXTMAKER_H

#include "pandatoolbase.h"
#include "pmap.h"
#include "namable.h"

#include <ft2build.h>
#include FT_FREETYPE_H

class Filename;
class PNMImage;
class TextGlyph;

////////////////////////////////////////////////////////////////////
//       Class : TextMaker
// Description : An object that uses Freetype to generate text at a
//               fixed pixel size into a PNMImage.
////////////////////////////////////////////////////////////////////
class TextMaker : public Namable {
public:
  TextMaker(const Filename &font_filename, int face_index);
  TextMaker(const char *font_data, int font_data_size, int face_index);
  ~TextMaker();

  bool is_valid() const;

  void set_pixel_size(int pixel_size, double scale_factor = 1.0);

  void generate_into(const string &text,
                     PNMImage &dest_image, int x, int y);

private:
  TextGlyph *get_glyph(int character);
  TextGlyph *make_glyph(int glyph_index);
  bool reset_scale(int pixel_size, double scale_factor);
  void empty_cache();

  static void initialize_ft_library();

  bool _is_valid;

  typedef pmap<int, TextGlyph *> Glyphs;
  Glyphs _glyphs;

  FT_Face _face;
  double _scale_factor;
  double _line_height;

  static FT_Library _ft_library;
  static bool _ft_initialized;
  static bool _ft_ok;
};

#endif
