// Filename: textNode.h
// Created by:  drose (13Mar02)
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

#ifndef TEXTNODE_H
#define TEXTNODE_H

#include "pandabase.h"

#include "config_text.h"
#include "textFont.h"
#include "pandaNode.h"

#include "luse.h"

class StringDecoder;

////////////////////////////////////////////////////////////////////
//       Class : TextNode
// Description : The primary interface to this module.  This class
//               does basic text assembly; given a string of text and
//               a TextFont object, it creates a piece of geometry
//               that may be placed in the 3-d or 2-d world to
//               represent the indicated text.
//
//               The TextNode may be used in one of two ways.
//               Naively, it may simply be parented directly into the
//               scene graph and rendered as if it were a GeomNode; in
//               this mode, the actual polygon geometry that renders
//               the text is not directly visible or accessible, but
//               remains hidden within the TextNode.
//
//               The second way TextNode may be used is as a text
//               generator.  To use it in this way, do not parent the
//               TextNode to the scene graph; instead, set the
//               properties of the text and call generate() to return
//               an ordinary node, containing ordinary geometry, which
//               you may use however you like.  Each time you call
//               generate() a new node is returned.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TextNode : public PandaNode {
PUBLISHED:
  TextNode(const string &name);
  ~TextNode();

  enum Alignment {
    A_left,
    A_right,
    A_center,
  };

  enum Encoding {
    E_iso8859,
    E_utf8,
    E_unicode
  };
 
  INLINE int freeze();
  INLINE int thaw();

  INLINE void set_font(TextFont *font);
  INLINE TextFont *get_font() const;

  INLINE static void set_default_font(TextFont *);
  INLINE static TextFont *get_default_font();

  INLINE void set_encoding(Encoding encoding);
  INLINE Encoding get_encoding() const;

  INLINE static void set_default_encoding(Encoding encoding);
  INLINE static Encoding get_default_encoding();

  INLINE void set_expand_amp(bool expand_amp);
  INLINE bool get_expand_amp() const;

  INLINE float get_line_height() const;

  INLINE void set_slant(float slant);
  INLINE float get_slant() const;

  INLINE void set_align(Alignment align_type);
  INLINE Alignment get_align() const;

  INLINE void set_wordwrap(float width);
  INLINE void clear_wordwrap();
  INLINE bool has_wordwrap() const;
  INLINE float get_wordwrap() const;

  INLINE void set_text_color(float r, float g, float b, float a);
  INLINE void set_text_color(const Colorf &text_color);
  INLINE void clear_text_color();
  INLINE bool has_text_color() const;
  INLINE Colorf get_text_color() const;

  INLINE void set_frame_color(float r, float g, float b, float a);
  INLINE void set_frame_color(const Colorf &frame_color);
  INLINE Colorf get_frame_color() const;

  INLINE void set_card_border(float size, float uv_portion);
  INLINE void clear_card_border();
  INLINE float get_card_border_size() const;
  INLINE float get_card_border_uv_portion() const;
  INLINE bool has_card_border() const;

  INLINE void set_card_color(float r, float g, float b, float a);
  INLINE void set_card_color(const Colorf &card_color);
  INLINE Colorf get_card_color() const;

  INLINE void set_card_texture(Texture *card_texture);
  INLINE void clear_card_texture();
  INLINE bool has_card_texture() const;
  INLINE Texture *get_card_texture() const;

  INLINE void set_shadow_color(float r, float g, float b, float a);
  INLINE void set_shadow_color(const Colorf &shadow_color);
  INLINE Colorf get_shadow_color() const;

  INLINE void set_frame_as_margin(float left, float right,
                                  float bottom, float top);
  INLINE void set_frame_actual(float left, float right,
                               float bottom, float top);
  INLINE void clear_frame();
  INLINE bool has_frame() const;
  INLINE bool is_frame_as_margin() const;
  INLINE LVecBase4f get_frame_as_set() const;
  INLINE LVecBase4f get_frame_actual() const;

  INLINE void set_frame_line_width(float line_width);
  INLINE float get_frame_line_width() const;
  INLINE void set_frame_corners(bool corners);
  INLINE bool get_frame_corners() const;

  INLINE void set_card_as_margin(float left, float right,
                                 float bottom, float top);
  INLINE void set_card_actual(float left, float right,
                              float bottom, float top);
  INLINE void clear_card();
  INLINE bool has_card() const;
  INLINE bool is_card_as_margin() const;
  INLINE LVecBase4f get_card_as_set() const;
  INLINE LVecBase4f get_card_actual() const;
  INLINE LVecBase4f get_card_transformed() const;

  INLINE void set_shadow(float xoffset, float yoffset);
  INLINE void clear_shadow();
  INLINE bool has_shadow() const;
  INLINE LVecBase2f get_shadow() const;

  INLINE void set_bin(const string &bin);
  INLINE void clear_bin();
  INLINE bool has_bin() const;
  INLINE const string &get_bin() const;

  INLINE int set_draw_order(int draw_order);
  INLINE int get_draw_order() const;

  INLINE void set_transform(const LMatrix4f &transform);
  INLINE LMatrix4f get_transform() const;

  INLINE void set_coordinate_system(CoordinateSystem cs);
  INLINE CoordinateSystem get_coordinate_system() const;

  INLINE void set_text(const string &text);
  INLINE void clear_text();
  INLINE bool has_text() const;
  INLINE string get_text() const;
  INLINE void append_text(const string &text);
  INLINE void append_char(int character);
  INLINE int get_num_chars() const;
  INLINE int get_char(int index) const;

  INLINE float calc_width(int character) const;
  INLINE float calc_width(const string &line) const;
  string wordwrap_to(const string &text, float wordwrap_width,
                     bool preserve_trailing_whitespace) const;

  virtual void write(ostream &out, int indent_level = 0) const;

  // The following functions return information about the text that
  // was last built (and is currently visible).
  INLINE float get_left() const;
  INLINE float get_right() const;
  INLINE float get_bottom() const;
  INLINE float get_top() const;
  INLINE float get_height() const;
  INLINE float get_width() const;

  INLINE LPoint3f get_upper_left_3d() const;
  INLINE LPoint3f get_lower_right_3d() const;

  INLINE int get_num_rows() const;

  PT(PandaNode) generate();
  INLINE void update();

public:
  // Direct support for wide-character strings.
  INLINE void set_wtext(const wstring &wtext);
  INLINE const wstring &get_wtext() const;
  INLINE void append_wtext(const wstring &text);

  INLINE float calc_width(const wstring &line) const;
  INLINE wstring wordwrap_to(const wstring &wtext, float wordwrap_width,
                             bool preserve_trailing_whitespace) const;

  string encode_wchar(wchar_t ch) const;
  string encode_wtext(const wstring &wtext) const;
  wstring decode_text(const string &text) const;

  // From parent class PandaNode
  virtual int get_unsafe_to_apply_attribs() const;
  virtual void apply_attribs_to_vertices(const AccumulatedAttribs &attribs,
                                         int attrib_types,
                                         GeomTransformer &transformer);
  virtual CPT(TransformState)
    calc_tight_bounds(LPoint3f &min_point, LPoint3f &max_point,
                      bool &found_any,
                      const TransformState *transform) const;

  virtual bool has_cull_callback() const;
  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data);

  virtual BoundingVolume *recompute_internal_bound();

private:
  wstring decode_text_impl(StringDecoder &decoder) const;
  int expand_amp_sequence(StringDecoder &decoder) const;

  INLINE void invalidate_no_measure();
  INLINE void invalidate_with_measure();
  INLINE void check_rebuild() const;
  INLINE void check_measure() const;

  void do_rebuild();
  void do_measure();

#ifndef CPPPARSER  // interrogate has a bit of trouble with wstring.
  float assemble_row(wstring::iterator &si, const wstring::iterator &send,
                     TextFont *font, PandaNode *dest);
  PT(PandaNode) assemble_text(wstring::iterator si, const wstring::iterator &send,
                              TextFont *font,
                              LVector2f &ul, LVector2f &lr, int &num_rows);
  float measure_row(wstring::iterator &si, const wstring::iterator &send,
                    TextFont *font);
  void measure_text(wstring::iterator si, const wstring::iterator &send,
                    TextFont *font,
                    LVector2f &ul, LVector2f &lr, int &num_rows);
#endif  // CPPPARSER

  PT(PandaNode) make_frame();
  PT(PandaNode) make_card();
  PT(PandaNode) make_card_with_border();

  static void load_default_font();

  PT(TextFont) _font;
  PT(PandaNode) _internal_geom;

  Encoding _encoding;
  float _slant;

  PT(Texture) _card_texture;
  Colorf _text_color;
  Colorf _shadow_color;
  Colorf _frame_color;
  Colorf _card_color;

  enum Flags {
    F_has_text_color   =  0x0001,
    F_has_wordwrap     =  0x0002,
    F_has_frame        =  0x0004,
    F_frame_as_margin  =  0x0008,
    F_has_card         =  0x0010,
    F_card_as_margin   =  0x0020,
    F_has_card_texture =  0x0040,
    F_has_shadow       =  0x0080,
    F_frame_corners    =  0x0100,
    F_card_transp      =  0x0200,
    F_has_card_border  =  0x0400,
    F_expand_amp       =  0x0800,
    F_got_text         =  0x1000,
    F_got_wtext        =  0x2000,
    F_needs_rebuild    =  0x4000,
    F_needs_measure    =  0x8000,
  };

  int _flags;
  Alignment _align;
  float _wordwrap_width;
  float _frame_width;
  float _card_border_size;
  float _card_border_uv_portion;

  LVector2f _frame_ul, _frame_lr;
  LVector2f _card_ul, _card_lr;
  LVector2f _shadow_offset;

  string _bin;
  int _draw_order;

  LMatrix4f _transform;
  CoordinateSystem _coordinate_system;

  string _text;
  wstring _wtext;

  LPoint2f _ul2d, _lr2d;
  LPoint3f _ul3d, _lr3d;
  int _num_rows;

  static PT(TextFont) _default_font;
  static bool _loaded_default_font;
  static Encoding _default_encoding;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PandaNode::init_type();
    register_type(_type_handle, "TextNode",
                  PandaNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "textNode.I"

#endif
