/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pgSliderBar.h
 * @author masad
 * @date 2004-10-19
 */

#ifndef PGSLIDERBAR_H
#define PGSLIDERBAR_H

#include "pandabase.h"

#include "pgItem.h"
#include "pgSliderBarNotify.h"
#include "pgButtonNotify.h"
#include "pgButton.h"

/**
 * This is a particular kind of PGItem that draws a little bar with a slider
 * that moves from left to right indicating a value between the ranges.
 *
 * This is used as an implementation for both DirectSlider and for
 * DirectScrollBar.
 */
class EXPCL_PANDA_PGUI PGSliderBar : public PGItem, public PGButtonNotify {
PUBLISHED:
  explicit PGSliderBar(const std::string &name = "");
  virtual ~PGSliderBar();

protected:
  PGSliderBar(const PGSliderBar &copy);

public:
  virtual PandaNode *make_copy() const;
  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data);
  virtual void xform(const LMatrix4 &mat);

  virtual void press(const MouseWatcherParameter &param, bool background);
  virtual void release(const MouseWatcherParameter &param, bool background);
  virtual void move(const MouseWatcherParameter &param);

  virtual void adjust();

  INLINE void set_notify(PGSliderBarNotify *notify);
  INLINE PGSliderBarNotify *get_notify() const;

PUBLISHED:
  void setup_scroll_bar(bool vertical, PN_stdfloat length, PN_stdfloat width, PN_stdfloat bevel);
  void setup_slider(bool vertical, PN_stdfloat length, PN_stdfloat width, PN_stdfloat bevel);

  INLINE void set_axis(const LVector3 &axis);
  INLINE const LVector3 &get_axis() const;
  MAKE_PROPERTY(axis, get_axis, set_axis);

  INLINE void set_range(PN_stdfloat min_value, PN_stdfloat max_value);
  INLINE PN_stdfloat get_min_value() const;
  INLINE PN_stdfloat get_max_value() const;

  INLINE void set_scroll_size(PN_stdfloat scroll_size);
  INLINE PN_stdfloat get_scroll_size() const;
  MAKE_PROPERTY(scroll_size, get_scroll_size, set_scroll_size);

  INLINE void set_page_size(PN_stdfloat page_size);
  INLINE PN_stdfloat get_page_size() const;
  MAKE_PROPERTY(page_size, get_page_size, set_page_size);

  INLINE void set_value(PN_stdfloat value);
  INLINE PN_stdfloat get_value() const;
  MAKE_PROPERTY(value, get_value, set_value);

  INLINE void set_ratio(PN_stdfloat ratio);
  INLINE PN_stdfloat get_ratio() const;
  MAKE_PROPERTY(ratio, get_ratio, set_ratio);

  INLINE bool is_button_down() const;
  MAKE_PROPERTY(button_down, is_button_down);

  INLINE void set_resize_thumb(bool resize_thumb);
  INLINE bool get_resize_thumb() const;
  MAKE_PROPERTY(resize_thumb, get_resize_thumb, set_resize_thumb);

  INLINE void set_manage_pieces(bool manage_pieces);
  INLINE bool get_manage_pieces() const;
  MAKE_PROPERTY(manage_pieces, get_manage_pieces, set_manage_pieces);

  INLINE void set_thumb_button(PGButton *thumb_button);
  INLINE void clear_thumb_button();
  INLINE PGButton *get_thumb_button() const;
  MAKE_PROPERTY2(thumb_button,
                 get_thumb_button, get_thumb_button,
                 set_thumb_button, clear_thumb_button);

  INLINE void set_left_button(PGButton *left_button);
  INLINE void clear_left_button();
  INLINE PGButton *get_left_button() const;
  MAKE_PROPERTY2(left_button,
                 get_left_button, get_left_button,
                 set_left_button, clear_left_button);

  INLINE void set_right_button(PGButton *right_button);
  INLINE void clear_right_button();
  INLINE PGButton *get_right_button() const;
  MAKE_PROPERTY2(right_button,
                 get_right_button, get_right_button,
                 set_right_button, clear_right_button);

  INLINE static std::string get_adjust_prefix();
  INLINE std::string get_adjust_event() const;
  MAKE_PROPERTY(adjust_prefix, get_adjust_prefix);

  virtual void set_active(bool active) final;
  MAKE_PROPERTY(active, get_active, set_active);

  void remanage();
  void recompute();

protected:
  virtual void frame_changed();

  virtual void item_transform_changed(PGItem *item);
  virtual void item_frame_changed(PGItem *item);
  virtual void item_draw_mask_changed(PGItem *item);
  virtual void item_press(PGItem *item, const MouseWatcherParameter &param);
  virtual void item_release(PGItem *item, const MouseWatcherParameter &param);
  virtual void item_move(PGItem *item, const MouseWatcherParameter &param);

private:
  INLINE void internal_set_ratio(PN_stdfloat ratio);

  void reposition();
  void advance_scroll();
  void advance_page();
  void begin_drag();
  void continue_drag();
  void end_drag();

private:
  bool _needs_remanage;
  bool _needs_recompute;
  bool _needs_reposition;

  PN_stdfloat _min_value, _max_value;
  PN_stdfloat _scroll_value, _scroll_ratio;
  PN_stdfloat _page_value, _page_ratio;
  PN_stdfloat _ratio;
  bool _resize_thumb;
  bool _manage_pieces;

  LVector3 _axis;

  PT(PGButton) _thumb_button;
  PT(PGButton) _left_button;
  PT(PGButton) _right_button;

  PN_stdfloat _min_x, _max_x, _thumb_width, _range_x;
  LPoint3 _thumb_start;
  PGItem *_scroll_button_held;
  bool _mouse_button_page;
  LPoint2 _mouse_pos;
  double _next_advance_time;
  bool _dragging;
  PN_stdfloat _drag_start_x;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PGItem::init_type();
    register_type(_type_handle, "PGSliderBar",
                  PGItem::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class PGButton;
};

#include "pgSliderBar.I"

#endif
