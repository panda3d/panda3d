// Filename: textFont.h
// Created by:  drose (03May01)
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

#include <pandabase.h>

#include "config_text.h"

#include <typedReferenceCount.h>
#include <namable.h>
#include <pt_Node.h>
#include <allTransitionsWrapper.h>

#include "pmap.h"

class Node;
class Geom;
class GeomPoint;

////////////////////////////////////////////////////////////////////
//       Class : TextFont
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TextFont : public TypedReferenceCount, public Namable {
PUBLISHED:
  TextFont(Node *font_def);
  ~TextFont();

  INLINE float get_line_height() const;

  float calc_width(char ch) const;
  float calc_width(const string &line) const;
  string wordwrap_to(const string &text, float wordwrap_width,
                     bool preserve_trailing_whitespace) const;

  void write(ostream &out, int indent_level) const;

private:
  // Private interfaces for the benefit of TextNode.
  class CharDef {
  public:
    CharDef() { }
    CharDef(Geom *geom, float width, const AllTransitionsWrapper &trans);
    Geom *_geom;
    float _width;
    AllTransitionsWrapper _trans;
  };

  INLINE const CharDef *get_char(int character) const;

private:
  bool find_character_gsets(Node *root, Geom *&ch, GeomPoint *&dot,
                            AllTransitionsWrapper &trans);
  void find_characters(Node *root);

  typedef pmap<int, CharDef> CharDefs;
  CharDefs _defs;
  float _font_height;
  PT_Node _font;

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

  friend class TextNode;
};

#include "textFont.I"

#endif
