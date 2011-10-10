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

public:
  virtual ~DynamicTextGlyph();

  INLINE bool intersects(int x, int y, int x_size, int y_size) const;
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
};

#include "dynamicTextGlyph.I"

#endif  // HAVE_FREETYPE

#endif
