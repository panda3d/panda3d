/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dynamicTextPage.h
 * @author drose
 * @date 2002-02-09
 */

#ifndef DYNAMICTEXTPAGE_H
#define DYNAMICTEXTPAGE_H

#include "pandabase.h"

#ifdef HAVE_FREETYPE

#include "texture.h"
#include "dynamicTextGlyph.h"
#include "pointerTo.h"
#include "pvector.h"

class DynamicTextFont;

/**
 * A single "page" of a DynamicTextFont.  This is a single texture that holds
 * a number of glyphs for rendering.  The font starts out with one page, and
 * will add more as it needs them.
 */
class EXPCL_PANDA_TEXT DynamicTextPage : public Texture {
public:
  DynamicTextPage(DynamicTextFont *font, int page_number);

  DynamicTextGlyph *slot_glyph(int character,  int x_size, int y_size,
                               int margin, PN_stdfloat advance);

PUBLISHED:
  INLINE const LVecBase2i &get_size() const;
  INLINE int get_x_size() const;
  INLINE int get_y_size() const;

  INLINE bool is_empty() const;

public:
  void fill_region(int x, int y, int x_size, int y_size, const LColor &color);

private:
  int garbage_collect(DynamicTextFont *font);

  bool find_hole(int &x, int &y, int x_size, int y_size) const;
  DynamicTextGlyph *find_overlap(int x, int y, int x_size, int y_size) const;

  typedef pvector< PT(DynamicTextGlyph) > Glyphs;
  Glyphs _glyphs;

  LVecBase2i _size;

  DynamicTextFont *_font;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    Texture::init_type();
    register_type(_type_handle, "DynamicTextPage",
                  Texture::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class DynamicTextFont;
};

#include "dynamicTextPage.I"

#endif  // HAVE_FREETYPE

#endif
