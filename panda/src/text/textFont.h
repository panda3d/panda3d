// Filename: textFont.h
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

#ifndef TEXTFONT_H
#define TEXTFONT_H

#include "pandabase.h"

#include "config_text.h"
#include "typedReferenceCount.h"
#include "namable.h"
#include "pmap.h"

class Node;
class TextGlyph;

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
class EXPCL_PANDA TextFont : public TypedReferenceCount, public Namable {
public:
  TextFont();

PUBLISHED:
  virtual ~TextFont();

  INLINE float get_line_height() const;

  float calc_width(int ch) const;
  float calc_width(const string &line) const;
  string wordwrap_to(const string &text, float wordwrap_width,
                     bool preserve_trailing_whitespace) const;

  virtual void write(ostream &out, int indent_level) const;

public:
  virtual const TextGlyph *get_glyph(int character) const=0;

protected:
  float _line_height;

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

#include "textFont.I"

#endif
