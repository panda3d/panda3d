/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textFont.h
 * @author drose
 * @date 2002-02-08
 */

#ifndef TEXTFONT_H
#define TEXTFONT_H

#include "pandabase.h"

#include "textGlyph.h"
#include "typedReferenceCount.h"
#include "namable.h"
#include "pmap.h"
#include "pointerTo.h"

/**
 * An encapsulation of a font; i.e.  a set of glyphs that may be assembled
 * together by a TextNode to represent a string of text.
 *
 * This is just an abstract interface; see StaticTextFont or DynamicTextFont
 * for an actual implementation.
 */
class EXPCL_PANDA_TEXT TextFont : public TypedReferenceCount, public Namable {
public:
  TextFont();
  TextFont(const TextFont &copy);

PUBLISHED:
  virtual ~TextFont();

  enum RenderMode {
    // Each glyph is a single textured rectangle
    RM_texture,

    // Each glyph is a lot of line segments
    RM_wireframe,

    // Each glyph is a lot of triangles
    RM_polygon,

    // a 3-D outline, like a cookie cutter
    RM_extruded,

    // combination of RM_extruded and RM_polygon
    RM_solid,

    RM_distance_field,

    // Returned by string_render_mode() for an invalid match.
    RM_invalid,
  };

  virtual PT(TextFont) make_copy() const=0;

  INLINE bool is_valid() const;
  INLINE operator bool () const;
  INLINE PN_stdfloat get_line_height() const;
  INLINE void set_line_height(PN_stdfloat line_height);
  MAKE_PROPERTY(valid, is_valid);
  MAKE_PROPERTY(line_height, get_line_height, set_line_height);

  INLINE PN_stdfloat get_space_advance() const;
  INLINE void set_space_advance(PN_stdfloat space_advance);
  MAKE_PROPERTY(space_advance, get_space_advance, set_space_advance);

  INLINE CPT(TextGlyph) get_glyph(int character);

  virtual PN_stdfloat get_kerning(int first, int second) const;

  virtual void write(std::ostream &out, int indent_level) const;

public:
  INLINE PN_stdfloat get_total_poly_margin() const;

  virtual bool get_glyph(int character, CPT(TextGlyph) &glyph)=0;
  TextGlyph *get_invalid_glyph();

  static RenderMode string_render_mode(const std::string &string);

private:
  void make_invalid_glyph();

protected:
  bool _is_valid;
  PN_stdfloat _line_height;
  PN_stdfloat _space_advance;
  PN_stdfloat _total_poly_margin;
  PT(TextGlyph) _invalid_glyph;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "TextFont",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

EXPCL_PANDA_TEXT std::ostream &operator << (std::ostream &out, TextFont::RenderMode rm);
EXPCL_PANDA_TEXT std::istream &operator >> (std::istream &in, TextFont::RenderMode &rm);

#include "textFont.I"

#endif
