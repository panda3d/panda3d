// Filename: textProperties.h
// Created by:  drose (06Apr04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef TEXTPROPERTIES_H
#define TEXTPROPERTIES_H

#include "pandabase.h"

#include "config_text.h"
#include "luse.h"
#include "textFont.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : TextProperties
// Description : This defines the set of visual properties that may be
//               assigned to the individual characters of the text.
//               (Properties which affect the overall block of text
//               can only be specified on the TextNode directly).
//
//               Typically, there is just one set of properties on a
//               given block of text, which is set directly on the
//               TextNode (TextNode inherits from TextProperties).
//               That makes all of the text within a particular block
//               have the same appearance.
//
//               This separate class exists in order to implement
//               multiple different kinds of text appearing within one
//               block.  The text string itself may reference a
//               TextProperties structure by name using the \1 and \2
//               tokens embedded within the string; each nested
//               TextProperties structure modifies the appearance of
//               subsequent text within the block.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TextProperties {
PUBLISHED:
  enum Alignment {
    A_left,
    A_right,
    A_center,
  };

  TextProperties();
  TextProperties(const TextProperties &copy);
  void operator = (const TextProperties &copy);

  void clear();
  INLINE bool is_any_specified() const;

  INLINE static void set_default_font(TextFont *);
  INLINE static TextFont *get_default_font();

  INLINE void set_font(TextFont *font);
  INLINE void clear_font();
  INLINE bool has_font() const;
  INLINE TextFont *get_font() const;

  INLINE void set_small_caps(bool small_caps);
  INLINE void clear_small_caps();
  INLINE bool has_small_caps() const;
  INLINE bool get_small_caps() const;

  INLINE void set_small_caps_scale(float small_caps_scale);
  INLINE void clear_small_caps_scale();
  INLINE bool has_small_caps_scale() const;
  INLINE float get_small_caps_scale() const;

  INLINE void set_slant(float slant);
  INLINE void clear_slant();
  INLINE bool has_slant() const;
  INLINE float get_slant() const;

  INLINE void set_align(Alignment align_type);
  INLINE void clear_align();
  INLINE bool has_align() const;
  INLINE Alignment get_align() const;

  INLINE void set_indent(float indent);
  INLINE void clear_indent();
  INLINE bool has_indent() const;
  INLINE float get_indent() const;

  INLINE void set_wordwrap(float wordwrap);
  INLINE void clear_wordwrap();
  INLINE bool has_wordwrap() const;
  INLINE float get_wordwrap() const;

  INLINE void set_preserve_trailing_whitespace(bool preserve_trailing_whitespace);
  INLINE void clear_preserve_trailing_whitespace();
  INLINE bool has_preserve_trailing_whitespace() const;
  INLINE bool get_preserve_trailing_whitespace() const;

  INLINE void set_text_color(float r, float g, float b, float a);
  INLINE void set_text_color(const Colorf &text_color);
  INLINE void clear_text_color();
  INLINE bool has_text_color() const;
  INLINE Colorf get_text_color() const;

  INLINE void set_shadow_color(float r, float g, float b, float a);
  INLINE void set_shadow_color(const Colorf &shadow_color);
  INLINE void clear_shadow_color();
  INLINE bool has_shadow_color() const;
  INLINE Colorf get_shadow_color() const;

  INLINE void set_shadow(float xoffset, float yoffset);
  INLINE void set_shadow(const LVecBase2f &shadow_offset);
  INLINE void clear_shadow();
  INLINE bool has_shadow() const;
  INLINE LVector2f get_shadow() const;

  INLINE void set_bin(const string &bin);
  INLINE void clear_bin();
  INLINE bool has_bin() const;
  INLINE const string &get_bin() const;

  INLINE int set_draw_order(int draw_order);
  INLINE void clear_draw_order();
  INLINE bool has_draw_order() const;
  INLINE int get_draw_order() const;

  INLINE void set_tab_width(float tab_width);
  INLINE void clear_tab_width();
  INLINE bool has_tab_width() const;
  INLINE float get_tab_width() const;

  INLINE void set_glyph_scale(float glyph_scale);
  INLINE void clear_glyph_scale();
  INLINE bool has_glyph_scale() const;
  INLINE float get_glyph_scale() const;

  INLINE void set_glyph_shift(float glyph_shift);
  INLINE void clear_glyph_shift();
  INLINE bool has_glyph_shift() const;
  INLINE float get_glyph_shift() const;

  void add_properties(const TextProperties &other);

  void write(ostream &out, int indent_level = 0) const;

private:
  static void load_default_font();

  enum Flags {
    F_has_font                         = 0x0001,
    F_has_small_caps                   = 0x0002,
    F_has_small_caps_scale             = 0x0004,
    F_has_slant                        = 0x0008,
    F_has_align                        = 0x0010,
    F_has_indent                       = 0x0020,
    F_has_wordwrap                     = 0x0040,
    F_has_preserve_trailing_whitespace = 0x0080,
    F_has_text_color                   = 0x0100,
    F_has_shadow_color                 = 0x0200,
    F_has_shadow                       = 0x0400,
    F_has_bin                          = 0x0800,
    F_has_draw_order                   = 0x1000,
    F_has_tab_width                    = 0x2000,
    F_has_glyph_scale                  = 0x4000,
    F_has_glyph_shift                  = 0x8000,
  };

  int _specified;

  PT(TextFont) _font;
  bool _small_caps;
  float _small_caps_scale;
  float _slant;
  Alignment _align;
  float _indent_width;
  float _wordwrap_width;
  bool _preserve_trailing_whitespace;
  Colorf _text_color;
  Colorf _shadow_color;
  LVector2f _shadow_offset;
  string _bin;
  int _draw_order;
  float _tab_width;
  float _glyph_scale;
  float _glyph_shift;

  static PT(TextFont) _default_font;
  static bool _loaded_default_font;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "TextProperties");
  }

private:
  static TypeHandle _type_handle;
};

#include "textProperties.I"

#endif
