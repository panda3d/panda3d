/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textNode.h
 * @author drose
 * @date 2002-03-13
 */

#ifndef TEXTNODE_H
#define TEXTNODE_H

#include "pandabase.h"

#include "config_text.h"
#include "textEncoder.h"
#include "textProperties.h"
#include "textFont.h"
#include "textAssembler.h"
#include "pandaNode.h"
#include "luse.h"
#include "geom.h"
#include "pmutex.h"
#include "mutexHolder.h"

/**
 * The primary interface to this module.  This class does basic text assembly;
 * given a string of text and a TextFont object, it creates a piece of
 * geometry that may be placed in the 3-d or 2-d world to represent the
 * indicated text.
 *
 * The TextNode may be used in one of two ways.  Naively, it may simply be
 * parented directly into the scene graph and rendered as if it were a
 * GeomNode; in this mode, the actual polygon geometry that renders the text
 * is not directly visible or accessible, but remains hidden within the
 * TextNode.
 *
 * The second way TextNode may be used is as a text generator.  To use it in
 * this way, do not parent the TextNode to the scene graph; instead, set the
 * properties of the text and call generate() to return an ordinary node,
 * containing ordinary geometry, which you may use however you like.  Each
 * time you call generate() a new node is returned.
 */
class EXPCL_PANDA_TEXT TextNode : public PandaNode, public TextEncoder, public TextProperties {
PUBLISHED:
  explicit TextNode(const std::string &name);
  explicit TextNode(const std::string &name, const TextProperties &copy);
protected:
  TextNode(const TextNode &copy);
  virtual PandaNode *make_copy() const;

PUBLISHED:
  ~TextNode();

  enum FlattenFlags {
    FF_none          = 0x0000,
    FF_light         = 0x0001,
    FF_medium        = 0x0002,
    FF_strong        = 0x0004,
    FF_dynamic_merge = 0x0008,
  };

  INLINE PN_stdfloat get_line_height() const;

  INLINE void set_max_rows(int max_rows);
  INLINE void clear_max_rows();
  INLINE bool has_max_rows() const;
  INLINE int get_max_rows() const;
  INLINE bool has_overflow() const;

  INLINE void set_frame_color(PN_stdfloat r, PN_stdfloat g, PN_stdfloat b, PN_stdfloat a);
  INLINE void set_frame_color(const LColor &frame_color);
  INLINE LColor get_frame_color() const;

  INLINE void set_card_border(PN_stdfloat size, PN_stdfloat uv_portion);
  INLINE void clear_card_border();
  INLINE PN_stdfloat get_card_border_size() const;
  INLINE PN_stdfloat get_card_border_uv_portion() const;
  INLINE bool has_card_border() const;

  INLINE void set_card_color(PN_stdfloat r, PN_stdfloat g, PN_stdfloat b, PN_stdfloat a);
  INLINE void set_card_color(const LColor &card_color);
  INLINE LColor get_card_color() const;

  INLINE void set_card_texture(Texture *card_texture);
  INLINE void clear_card_texture();
  INLINE bool has_card_texture() const;
  INLINE Texture *get_card_texture() const;

  INLINE void set_frame_as_margin(PN_stdfloat left, PN_stdfloat right,
                                  PN_stdfloat bottom, PN_stdfloat top);
  INLINE void set_frame_actual(PN_stdfloat left, PN_stdfloat right,
                               PN_stdfloat bottom, PN_stdfloat top);
  INLINE void clear_frame();
  INLINE bool has_frame() const;
  INLINE bool is_frame_as_margin() const;
  INLINE LVecBase4 get_frame_as_set() const;
  INLINE LVecBase4 get_frame_actual() const;

  INLINE void set_frame_line_width(PN_stdfloat line_width);
  INLINE PN_stdfloat get_frame_line_width() const;
  INLINE void set_frame_corners(bool corners);
  INLINE bool get_frame_corners() const;

  INLINE void set_card_as_margin(PN_stdfloat left, PN_stdfloat right,
                                 PN_stdfloat bottom, PN_stdfloat top);
  INLINE void set_card_actual(PN_stdfloat left, PN_stdfloat right,
                              PN_stdfloat bottom, PN_stdfloat top);
  INLINE void set_card_decal(bool card_decal);
  INLINE void clear_card();
  INLINE bool has_card() const;
  INLINE bool get_card_decal() const;
  INLINE bool is_card_as_margin() const;
  INLINE LVecBase4 get_card_as_set() const;
  INLINE LVecBase4 get_card_actual() const;
  INLINE LVecBase4 get_card_transformed() const;

  INLINE void set_transform(const LMatrix4 &transform);
  INLINE LMatrix4 get_transform() const;

  INLINE void set_coordinate_system(CoordinateSystem cs);
  INLINE CoordinateSystem get_coordinate_system() const;

  INLINE void set_usage_hint(Geom::UsageHint usage_hint);
  INLINE Geom::UsageHint get_usage_hint() const;

  INLINE void set_flatten_flags(int flatten_flags);
  INLINE int get_flatten_flags() const;

  // These methods are inherited from TextProperties, but we override here so
  // we can flag the TextNode as dirty when they have been changed.

  INLINE void set_font(TextFont *font);
  INLINE void clear_font();

  INLINE void set_small_caps(bool small_caps);
  INLINE void clear_small_caps();

  INLINE void set_small_caps_scale(PN_stdfloat small_caps_scale);
  INLINE void clear_small_caps_scale();

  INLINE void set_slant(PN_stdfloat slant);
  INLINE void clear_slant();

  INLINE void set_align(Alignment align_type);
  INLINE void clear_align();

  INLINE void set_indent(PN_stdfloat indent);
  INLINE void clear_indent();

  INLINE void set_wordwrap(PN_stdfloat wordwrap);
  INLINE void clear_wordwrap();

  INLINE void set_text_color(PN_stdfloat r, PN_stdfloat g, PN_stdfloat b, PN_stdfloat a);
  INLINE void set_text_color(const LColor &text_color);
  INLINE void clear_text_color();

  INLINE void set_shadow_color(PN_stdfloat r, PN_stdfloat g, PN_stdfloat b, PN_stdfloat a);
  INLINE void set_shadow_color(const LColor &shadow_color);
  INLINE void clear_shadow_color();

  INLINE void set_shadow(PN_stdfloat xoffset, PN_stdfloat yoffset);
  INLINE void set_shadow(const LVecBase2 &shadow_offset);
  INLINE void clear_shadow();

  INLINE void set_bin(const std::string &bin);
  INLINE void clear_bin();

  INLINE int set_draw_order(int draw_order);
  INLINE void clear_draw_order();

  INLINE void set_tab_width(PN_stdfloat tab_width);
  INLINE void clear_tab_width();

  INLINE void set_glyph_scale(PN_stdfloat glyph_scale);
  INLINE void clear_glyph_scale();

  INLINE void set_glyph_shift(PN_stdfloat glyph_shift);
  INLINE void clear_glyph_shift();

  // After the text has been set, you can query this to determine how it will
  // be wordwrapped.
  INLINE std::string get_wordwrapped_text() const;

  // These methods calculate the width of a single character or a line of text
  // in the current font.
  PN_stdfloat calc_width(wchar_t character) const;
  INLINE PN_stdfloat calc_width(const std::string &line) const;

  bool has_exact_character(wchar_t character) const;
  bool has_character(wchar_t character) const;
  bool is_whitespace(wchar_t character) const;

  INLINE std::wstring get_wordwrapped_wtext() const;
  PN_stdfloat calc_width(const std::wstring &line) const;

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent_level = 0) const;

  // The following functions return information about the text that was last
  // built (and is currently visible).
  INLINE PN_stdfloat get_left() const;
  INLINE PN_stdfloat get_right() const;
  INLINE PN_stdfloat get_bottom() const;
  INLINE PN_stdfloat get_top() const;
  INLINE PN_stdfloat get_height() const;
  INLINE PN_stdfloat get_width() const;

  INLINE LPoint3 get_upper_left_3d() const;
  INLINE LPoint3 get_lower_right_3d() const;

  INLINE int get_num_rows() const;

  INLINE PT(PandaNode) generate();
  INLINE void update();
  INLINE void force_update();

  PT(PandaNode) get_internal_geom() const;

PUBLISHED:
  MAKE_PROPERTY(max_rows, get_max_rows, set_max_rows);
  MAKE_PROPERTY(frame_color, get_frame_color, set_frame_color);
  MAKE_PROPERTY(card_color, get_card_color, set_card_color);
  MAKE_PROPERTY(card_texture, get_card_texture, set_card_texture);
  MAKE_PROPERTY(frame_line_width, get_frame_line_width, set_frame_line_width);
  MAKE_PROPERTY(frame_corners, get_frame_corners, set_frame_corners);
  MAKE_PROPERTY(transform, get_transform, set_transform);
  MAKE_PROPERTY(coordinate_system, get_coordinate_system, set_coordinate_system);
  MAKE_PROPERTY(usage_hint, get_usage_hint, set_usage_hint);
  MAKE_PROPERTY(flatten_flags, get_flatten_flags, set_flatten_flags);

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

public:
  // From parent class TextEncoder;
  virtual void text_changed() final;

  // From parent class PandaNode
  virtual int get_unsafe_to_apply_attribs() const;
  virtual void apply_attribs_to_vertices(const AccumulatedAttribs &attribs,
                                         int attrib_types,
                                         GeomTransformer &transformer);
  virtual CPT(TransformState)
    calc_tight_bounds(LPoint3 &min_point, LPoint3 &max_point,
                      bool &found_any,
                      const TransformState *transform,
                      Thread *current_thread) const;

  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data);
  virtual bool is_renderable() const;

  virtual void compute_internal_bounds(CPT(BoundingVolume) &internal_bounds,
                                       int &internal_vertices,
                                       int pipeline_stage,
                                       Thread *current_thread) const;

  virtual void r_prepare_scene(GraphicsStateGuardianBase *gsg,
                               const RenderState *node_state,
                               GeomTransformer &transformer,
                               Thread *current_thread);

private:
  INLINE void invalidate_no_measure();
  INLINE void invalidate_with_measure();
  INLINE void check_rebuild() const;
  INLINE void check_measure() const;

  void do_rebuild();
  void do_measure();

  PT(PandaNode) do_generate();
  PT(PandaNode) do_get_internal_geom() const;

  PT(PandaNode) make_frame();
  PT(PandaNode) make_card();
  PT(PandaNode) make_card_with_border();

  static int count_geoms(PandaNode *node);

  Mutex _lock;
  PT(PandaNode) _internal_geom;

  PT(Texture) _card_texture;
  LColor _frame_color;
  LColor _card_color;

  enum Flags {
    F_has_frame        =  0x0001,
    F_frame_as_margin  =  0x0002,
    F_has_card         =  0x0004,
    F_card_as_margin   =  0x0008,
    F_has_card_texture =  0x0010,
    F_frame_corners    =  0x0020,
    F_card_transp      =  0x0040,
    F_has_card_border  =  0x0080,
    F_needs_rebuild    =  0x0100,
    F_needs_measure    =  0x0200,
    F_has_overflow     =  0x0400,
    F_card_decal       =  0x0800,
  };

  int _flags;
  int _max_rows;
  GeomEnums::UsageHint _usage_hint;
  int _flatten_flags;
  PN_stdfloat _frame_width;
  PN_stdfloat _card_border_size;
  PN_stdfloat _card_border_uv_portion;

  LVector2 _frame_ul, _frame_lr;
  LVector2 _card_ul, _card_lr;

  LMatrix4 _transform;
  CoordinateSystem _coordinate_system;

  LPoint3 _ul3d, _lr3d;

  // Returned from TextAssembler:
  LVector2 _text_ul, _text_lr;
  int _num_rows;
  std::wstring _wordwrapped_wtext;

  static PStatCollector _text_generate_pcollector;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PandaNode::init_type();
    TextProperties::init_type();
    register_type(_type_handle, "TextNode",
                  PandaNode::get_class_type(),
                  TextProperties::get_class_type());
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
