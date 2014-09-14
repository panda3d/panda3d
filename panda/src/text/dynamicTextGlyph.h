// Filename: dynamicTextGlyph.h
// Created by:  drose (09Feb02)
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

#ifndef DYNAMICTEXTGLYPH_H
#define DYNAMICTEXTGLYPH_H

#include "pandabase.h"

#ifdef HAVE_FREETYPE

#include "textGlyph.h"

class DynamicTextPage;
class DynamicTextFont;

////////////////////////////////////////////////////////////////////
//       Class : DynamicTextGlyph
// Description : A specialization on TextGlyph that is generated and
//               stored by a DynamicTextFont.  This keeps some
//               additional information, such as where the glyph
//               appears on a texture map.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_TEXT DynamicTextGlyph : public TextGlyph {
public:
  INLINE DynamicTextGlyph(int character, DynamicTextPage *page,
                          int x, int y, int x_size, int y_size, 
                          int margin);
  INLINE DynamicTextGlyph(int character, PN_stdfloat advance);
private:
  INLINE DynamicTextGlyph(const DynamicTextGlyph &copy);
  INLINE void operator = (const DynamicTextGlyph &copy);

PUBLISHED:
  virtual ~DynamicTextGlyph();

  INLINE DynamicTextPage *get_page() const;

  INLINE bool intersects(int x, int y, int x_size, int y_size) const;

  INLINE PN_stdfloat get_top() const;
  INLINE PN_stdfloat get_left() const;
  INLINE PN_stdfloat get_bottom() const;
  INLINE PN_stdfloat get_right() const;

  INLINE PN_stdfloat get_uv_top() const;
  INLINE PN_stdfloat get_uv_left() const;
  INLINE PN_stdfloat get_uv_bottom() const;
  INLINE PN_stdfloat get_uv_right() const;

public:
  unsigned char *get_row(int y);
  void erase(DynamicTextFont *font);
  void make_geom(int top, int left, PN_stdfloat advance, PN_stdfloat poly_margin,
                 PN_stdfloat tex_x_size, PN_stdfloat tex_y_size,
                 PN_stdfloat font_pixels_per_unit, PN_stdfloat tex_pixels_per_unit);
  void set_geom(GeomVertexData *vdata, GeomPrimitive *prim, 
                const RenderState *state);
  virtual bool is_whitespace() const;

  DynamicTextPage *_page;
  int _geom_count;

  int _x, _y;
  int _x_size, _y_size;
  int _margin;
  PN_stdfloat _top, _left, _bottom, _right;
  PN_stdfloat _uv_top, _uv_left, _uv_bottom, _uv_right;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TextGlyph::init_type();
    register_type(_type_handle, "DynamicTextGlyph",
                  TextGlyph::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "dynamicTextGlyph.I"

#endif  // HAVE_FREETYPE

#endif
