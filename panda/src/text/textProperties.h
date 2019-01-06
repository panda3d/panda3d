/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textProperties.h
 * @author drose
 * @date 2004-04-06
 */

#ifndef TEXTPROPERTIES_H
#define TEXTPROPERTIES_H

#include "pandabase.h"

#include "config_text.h"
#include "luse.h"
#include "textFont.h"
#include "pointerTo.h"
#include "renderState.h"

/**
 * This defines the set of visual properties that may be assigned to the
 * individual characters of the text.  (Properties which affect the overall
 * block of text can only be specified on the TextNode directly).
 *
 * Typically, there is just one set of properties on a given block of text,
 * which is set directly on the TextNode (TextNode inherits from
 * TextProperties). That makes all of the text within a particular block have
 * the same appearance.
 *
 * This separate class exists in order to implement multiple different kinds
 * of text appearing within one block.  The text string itself may reference a
 * TextProperties structure by name using the \1 and \2 tokens embedded within
 * the string; each nested TextProperties structure modifies the appearance of
 * subsequent text within the block.
 */
class EXPCL_PANDA_TEXT TextProperties {
PUBLISHED:
  enum Alignment {
    A_left,
    A_right,
    A_center,
    A_boxed_left,
    A_boxed_right,
    A_boxed_center
  };

  enum Direction {
    D_ltr,
    D_rtl,
  };

  TextProperties();
  TextProperties(const TextProperties &copy);
  void operator = (const TextProperties &copy);

  bool operator == (const TextProperties &other) const;
  INLINE bool operator != (const TextProperties &other) const;

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

  INLINE void set_small_caps_scale(PN_stdfloat small_caps_scale);
  INLINE void clear_small_caps_scale();
  INLINE bool has_small_caps_scale() const;
  INLINE PN_stdfloat get_small_caps_scale() const;

  INLINE void set_slant(PN_stdfloat slant);
  INLINE void clear_slant();
  INLINE bool has_slant() const;
  INLINE PN_stdfloat get_slant() const;

  INLINE void set_underscore(bool underscore);
  INLINE void clear_underscore();
  INLINE bool has_underscore() const;
  INLINE bool get_underscore() const;

  INLINE void set_underscore_height(PN_stdfloat underscore_height);
  INLINE void clear_underscore_height();
  INLINE bool has_underscore_height() const;
  INLINE PN_stdfloat get_underscore_height() const;

  INLINE void set_align(Alignment align_type);
  INLINE void clear_align();
  INLINE bool has_align() const;
  INLINE Alignment get_align() const;

  INLINE void set_indent(PN_stdfloat indent);
  INLINE void clear_indent();
  INLINE bool has_indent() const;
  INLINE PN_stdfloat get_indent() const;

  INLINE void set_wordwrap(PN_stdfloat wordwrap);
  INLINE void clear_wordwrap();
  INLINE bool has_wordwrap() const;
  INLINE PN_stdfloat get_wordwrap() const;

  INLINE void set_preserve_trailing_whitespace(bool preserve_trailing_whitespace);
  INLINE void clear_preserve_trailing_whitespace();
  INLINE bool has_preserve_trailing_whitespace() const;
  INLINE bool get_preserve_trailing_whitespace() const;

  INLINE void set_text_color(PN_stdfloat r, PN_stdfloat g, PN_stdfloat b, PN_stdfloat a);
  INLINE void set_text_color(const LColor &text_color);
  INLINE void clear_text_color();
  INLINE bool has_text_color() const;
  INLINE LColor get_text_color() const;

  INLINE void set_shadow_color(PN_stdfloat r, PN_stdfloat g, PN_stdfloat b, PN_stdfloat a);
  INLINE void set_shadow_color(const LColor &shadow_color);
  INLINE void clear_shadow_color();
  INLINE bool has_shadow_color() const;
  INLINE LColor get_shadow_color() const;

  INLINE void set_shadow(PN_stdfloat xoffset, PN_stdfloat yoffset);
  INLINE void set_shadow(const LVecBase2 &shadow_offset);
  INLINE void clear_shadow();
  INLINE bool has_shadow() const;
  INLINE LVector2 get_shadow() const;

  INLINE void set_bin(const std::string &bin);
  INLINE void clear_bin();
  INLINE bool has_bin() const;
  INLINE const std::string &get_bin() const;

  INLINE int set_draw_order(int draw_order);
  INLINE void clear_draw_order();
  INLINE bool has_draw_order() const;
  INLINE int get_draw_order() const;

  INLINE void set_tab_width(PN_stdfloat tab_width);
  INLINE void clear_tab_width();
  INLINE bool has_tab_width() const;
  INLINE PN_stdfloat get_tab_width() const;

  INLINE void set_glyph_scale(PN_stdfloat glyph_scale);
  INLINE void clear_glyph_scale();
  INLINE bool has_glyph_scale() const;
  INLINE PN_stdfloat get_glyph_scale() const;

  INLINE void set_glyph_shift(PN_stdfloat glyph_shift);
  INLINE void clear_glyph_shift();
  INLINE bool has_glyph_shift() const;
  INLINE PN_stdfloat get_glyph_shift() const;

  INLINE void set_text_scale(PN_stdfloat text_scale);
  INLINE void clear_text_scale();
  INLINE bool has_text_scale() const;
  INLINE PN_stdfloat get_text_scale() const;

  INLINE void set_direction(Direction direction);
  INLINE void clear_direction();
  INLINE bool has_direction() const;
  INLINE Direction get_direction() const;

  void add_properties(const TextProperties &other);

  void write(std::ostream &out, int indent_level = 0) const;

PUBLISHED:
  MAKE_PROPERTY2(font, has_font, get_font, set_font, clear_font);
  MAKE_PROPERTY2(small_caps, has_small_caps, get_small_caps,
                             set_small_caps, clear_small_caps);
  MAKE_PROPERTY2(small_caps_scale, has_small_caps_scale, get_small_caps_scale,
                                   set_small_caps_scale, clear_small_caps_scale);
  MAKE_PROPERTY2(slant, has_slant, get_slant, set_slant, clear_slant);
  MAKE_PROPERTY2(underscore, has_underscore, get_underscore,
                                 set_underscore, clear_underscore);
  MAKE_PROPERTY2(underscore_height, has_underscore_height, get_underscore_height,
                                    set_underscore_height, clear_underscore_height);
  MAKE_PROPERTY2(align, has_align, get_align, set_align, clear_align);
  MAKE_PROPERTY2(indent, has_indent, get_indent, set_indent, clear_indent);
  MAKE_PROPERTY2(wordwrap, has_wordwrap, get_wordwrap, set_wordwrap, clear_wordwrap);
  MAKE_PROPERTY2(preserve_trailing_whitespace,
                 has_preserve_trailing_whitespace, get_preserve_trailing_whitespace,
                 set_preserve_trailing_whitespace, clear_preserve_trailing_whitespace);
  MAKE_PROPERTY2(text_color, has_text_color, get_text_color,
                             set_text_color, clear_text_color);
  MAKE_PROPERTY2(shadow_color, has_shadow_color, get_shadow_color,
                               set_shadow_color, clear_shadow_color);
  MAKE_PROPERTY2(shadow, has_shadow, get_shadow, set_shadow, clear_shadow);
  MAKE_PROPERTY2(bin, has_bin, get_bin, set_bin, clear_bin);
  MAKE_PROPERTY2(draw_order, has_draw_order, get_draw_order,
                             set_draw_order, clear_draw_order);
  MAKE_PROPERTY2(tab_width, has_tab_width, get_tab_width,
                            set_tab_width, clear_tab_width);
  MAKE_PROPERTY2(glyph_scale, has_glyph_scale, get_glyph_scale,
                              set_glyph_scale, clear_glyph_scale);
  MAKE_PROPERTY2(glyph_shift, has_glyph_shift, get_glyph_shift,
                              set_glyph_shift, clear_glyph_shift);
  MAKE_PROPERTY2(text_scale, has_text_scale, get_text_scale,
                             set_text_scale, clear_text_scale);
  MAKE_PROPERTY2(direction, has_direction, get_direction,
                            set_direction, clear_direction);

public:
  const RenderState *get_text_state() const;
  const RenderState *get_shadow_state() const;

private:
  static void load_default_font();

  enum Flags {
    F_has_font                         = 0x00000001,
    F_has_small_caps                   = 0x00000002,
    F_has_small_caps_scale             = 0x00000004,
    F_has_slant                        = 0x00000008,
    F_has_align                        = 0x00000010,
    F_has_indent                       = 0x00000020,
    F_has_wordwrap                     = 0x00000040,
    F_has_preserve_trailing_whitespace = 0x00000080,
    F_has_text_color                   = 0x00000100,
    F_has_shadow_color                 = 0x00000200,
    F_has_shadow                       = 0x00000400,
    F_has_bin                          = 0x00000800,
    F_has_draw_order                   = 0x00001000,
    F_has_tab_width                    = 0x00002000,
    F_has_glyph_scale                  = 0x00004000,
    F_has_glyph_shift                  = 0x00008000,
    F_has_underscore                   = 0x00010000,
    F_has_underscore_height            = 0x00020000,
    F_has_text_scale                   = 0x00040000,
    F_has_direction                    = 0x00080000,
  };

  int _specified;

  PT(TextFont) _font;
  bool _small_caps;
  PN_stdfloat _small_caps_scale;
  PN_stdfloat _slant;
  bool _underscore;
  PN_stdfloat _underscore_height;
  Alignment _align;
  PN_stdfloat _indent_width;
  PN_stdfloat _wordwrap_width;
  bool _preserve_trailing_whitespace;
  LColor _text_color;
  LColor _shadow_color;
  LVector2 _shadow_offset;
  std::string _bin;
  int _draw_order;
  PN_stdfloat _tab_width;
  PN_stdfloat _glyph_scale;
  PN_stdfloat _glyph_shift;
  PN_stdfloat _text_scale;
  Direction _direction;

  mutable CPT(RenderState) _text_state;
  mutable CPT(RenderState) _shadow_state;

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
