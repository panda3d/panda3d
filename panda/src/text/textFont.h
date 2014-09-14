// Filename: textFont.h
// Created by:  drose (08Feb02)
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

#ifndef TEXTFONT_H
#define TEXTFONT_H

#include "pandabase.h"

#include "textGlyph.h"
#include "typedReferenceCount.h"
#include "namable.h"
#include "pmap.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : TextFont
// Description : An encapsulation of a font; i.e. a set of glyphs that
//               may be assembled together by a TextNode to represent
//               a string of text.
//
//               This is just an abstract interface; see
//               StaticTextFont or DynamicTextFont for an actual
//               implementation.
////////////////////////////////////////////////////////////////////
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

    // Returned by string_render_mode() for an invalid match.
    RM_invalid,
  };

  enum WindingOrder {
    WO_default,
    WO_left,
    WO_right,

    WO_invalid,
  };

  virtual PT(TextFont) make_copy() const=0;

  INLINE bool is_valid() const;
  INLINE operator bool () const;
  INLINE PN_stdfloat get_line_height() const;
  INLINE void set_line_height(PN_stdfloat line_height);

  INLINE PN_stdfloat get_space_advance() const;
  INLINE void set_space_advance(PN_stdfloat space_advance);
  INLINE const TextGlyph *get_glyph(int character);

  virtual void write(ostream &out, int indent_level) const;

public:
  virtual bool get_glyph(int character, const TextGlyph *&glyph)=0;
  TextGlyph *get_invalid_glyph();

  static RenderMode string_render_mode(const string &string);
  static WindingOrder string_winding_order(const string &string);

private:
  void make_invalid_glyph();

protected:
  bool _is_valid;
  PN_stdfloat _line_height;
  PN_stdfloat _space_advance;
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

EXPCL_PANDA_TEXT ostream &operator << (ostream &out, TextFont::RenderMode rm);
EXPCL_PANDA_TEXT istream &operator >> (istream &in, TextFont::RenderMode &rm);
EXPCL_PANDA_TEXT ostream &operator << (ostream &out, TextFont::WindingOrder wo);
EXPCL_PANDA_TEXT istream &operator >> (istream &in, TextFont::WindingOrder &wo);

#include "textFont.I"

#endif
