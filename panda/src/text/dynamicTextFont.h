// Filename: dynamicTextFont.h
// Created by:  drose (08Feb02)
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

#ifndef DYNAMICTEXTFONT_H
#define DYNAMICTEXTFONT_H

#include "pandabase.h"

#ifdef HAVE_FREETYPE

#include "config_text.h"
#include "textFont.h"
#include "dynamicTextGlyph.h"
#include "dynamicTextPage.h"
#include "filename.h"
#include "pvector.h"
#include "pmap.h"

#include <ft2build.h>
#include FT_FREETYPE_H

////////////////////////////////////////////////////////////////////
//       Class : DynamicTextFont
// Description : A DynamicTextFont is a special TextFont object that
//               rasterizes its glyphs from a standard font file
//               (e.g. a TTF file) on the fly.  It requires the
//               FreeType 2.0 library (or any higher,
//               backward-compatible version).
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA DynamicTextFont : public TextFont {
PUBLISHED:
  DynamicTextFont(const Filename &font_filename, int face_index = 0);

  INLINE bool set_point_size(float point_size);
  INLINE float get_point_size() const;

  INLINE bool set_pixels_per_unit(float pixels_per_unit);
  INLINE float get_pixels_per_unit() const;

  INLINE void set_texture_margin(int texture_margin);
  INLINE int get_texture_margin() const;
  INLINE void set_poly_margin(float poly_margin);
  INLINE float get_poly_margin() const;

  INLINE void set_page_size(int x_size, int y_size);
  INLINE int get_page_x_size() const;
  INLINE int get_page_y_size() const;

  int get_num_pages() const;
  DynamicTextPage *get_page(int n) const;

  void clear();

  virtual void write(ostream &out, int indent_level) const;

public:
  virtual const TextGlyph *get_glyph(int character);

private:
  bool reset_scale();
  DynamicTextGlyph *make_glyph(int character);
  DynamicTextGlyph *slot_glyph(int x_size, int y_size);

  static void initialize_ft_library();

  float _point_size;
  float _pixels_per_unit;
  int _texture_margin;
  float _poly_margin;
  int _page_x_size, _page_y_size;

  typedef pvector< PT(DynamicTextPage) > Pages;
  Pages _pages;

  typedef pmap<int, DynamicTextGlyph *> Cache;
  Cache _cache;

  FT_Face _face;

  static FT_Library _ft_library;
  static bool _ft_initialized;
  static bool _ft_ok;

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

#include "dynamicTextFont.I"

#endif  // HAVE_FREETYPE

#endif
