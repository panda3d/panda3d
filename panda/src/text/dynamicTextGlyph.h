/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dynamicTextGlyph.h
 * @author drose
 * @date 2002-02-09
 */

#ifndef DYNAMICTEXTGLYPH_H
#define DYNAMICTEXTGLYPH_H

#include "pandabase.h"

#ifdef HAVE_FREETYPE

#include "textGlyph.h"

class DynamicTextPage;
class DynamicTextFont;

/**
 * A specialization on TextGlyph that is generated and stored by a
 * DynamicTextFont.  This keeps some additional information, such as where the
 * glyph appears on a texture map.
 */
class EXPCL_PANDA_TEXT DynamicTextGlyph : public TextGlyph {
public:
  INLINE DynamicTextGlyph(int character, DynamicTextPage *page,
                          int x, int y, int x_size, int y_size,
                          int margin, PN_stdfloat advance);
  INLINE DynamicTextGlyph(int character, PN_stdfloat advance);
  DynamicTextGlyph(const DynamicTextGlyph &copy) = delete;

  DynamicTextGlyph &operator = (const DynamicTextGlyph &copy) = delete;

PUBLISHED:
  virtual ~DynamicTextGlyph();

  INLINE DynamicTextPage *get_page() const;
  MAKE_PROPERTY(page, get_page);

  INLINE bool intersects(int x, int y, int x_size, int y_size) const;

  INLINE PN_stdfloat get_left() const;
  INLINE PN_stdfloat get_bottom() const;
  INLINE PN_stdfloat get_right() const;
  INLINE PN_stdfloat get_top() const;

  INLINE PN_stdfloat get_uv_left() const;
  INLINE PN_stdfloat get_uv_bottom() const;
  INLINE PN_stdfloat get_uv_right() const;
  INLINE PN_stdfloat get_uv_top() const;

public:
  unsigned char *get_row(int y);
  void erase(DynamicTextFont *font);
  virtual bool is_whitespace() const;

  DynamicTextPage *_page;

  int _x, _y;
  int _x_size, _y_size;
  int _margin;

  friend class DynamicTextFont;

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
