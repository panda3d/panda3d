// Filename: dynamicTextGlyph.h
// Created by:  drose (09Feb02)
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

#ifndef DYNAMICTEXTGLYPH_H
#define DYNAMICTEXTGLYPH_H

#include "pandabase.h"

#ifdef HAVE_FREETYPE

#include "textGlyph.h"

class DynamicTextPage;

////////////////////////////////////////////////////////////////////
//       Class : DynamicTextGlyph
// Description : A specialization on TextGlyph that is generated and
//               stored by a DynamicTextFont.  This keeps some
//               additional information, such as where the glyph
//               appears on a texture map.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA DynamicTextGlyph : public TextGlyph {
public:
  INLINE DynamicTextGlyph(DynamicTextPage *page, int x, int y,
                          int x_size, int y_size, int margin);
  INLINE DynamicTextGlyph(float advance);
private:
  INLINE DynamicTextGlyph(const DynamicTextGlyph &copy);
  INLINE void operator = (const DynamicTextGlyph &copy);

public:
  virtual ~DynamicTextGlyph();

  INLINE bool intersects(int x, int y, int x_size, int y_size) const;
  unsigned char *get_row(int y);
  void erase();
  void make_geom(int top, int left, float advance, float poly_margin,
                 float tex_x_size, float tex_y_size,
                 float font_pixels_per_unit, float tex_pixels_per_unit);

  DynamicTextPage *_page;
  int _geom_count;

  int _x, _y;
  int _x_size, _y_size;
  int _margin;
};

#include "dynamicTextGlyph.I"

#endif  // HAVE_FREETYPE

#endif
