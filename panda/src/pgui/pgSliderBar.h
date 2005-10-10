// Filename: pgSliderBar.h
// Created by:  masad (19Oct04)
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

#ifndef PGSLIDERBAR_H
#define PGSLIDERBAR_H

#include "pandabase.h"

#include "pgItem.h"
#include "pgSliderBarNotify.h"
#include "pgButtonNotify.h"
#include "pgButton.h"

////////////////////////////////////////////////////////////////////
//       Class : PGSliderBar
// Description : This is a particular kind of PGItem that draws a
//               little bar with a slider that moves from left to 
//               right indicating a value between the ranges.
//
//               This is used as an implementation for both
//               DirectSlider and for DirectScrollBar.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PGSliderBar : public PGItem, public PGButtonNotify {
PUBLISHED:
  PGSliderBar(const string &name = "");
  virtual ~PGSliderBar();

protected:
  PGSliderBar(const PGSliderBar &copy);

public:
  virtual PandaNode *make_copy() const;
  virtual bool has_cull_callback() const;
  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data);
  virtual void xform(const LMatrix4f &mat);

  virtual void press(const MouseWatcherParameter &param, bool background);
  virtual void release(const MouseWatcherParameter &param, bool background);
  virtual void move(const MouseWatcherParameter &param);

  virtual void adjust();

  INLINE void set_notify(PGSliderBarNotify *notify);
  INLINE PGSliderBarNotify *get_notify() const;

PUBLISHED:
  void setup_scroll_bar(bool vertical, float length, float width, float bevel);
  void setup_slider(bool vertical, float length, float width, float bevel);

  INLINE void set_axis(const LVector3f &axis);
  INLINE const LVector3f &get_axis() const;

  INLINE void set_range(float min_value, float max_value);
  INLINE float get_min_value() const;
  INLINE float get_max_value() const;

  INLINE void set_scroll_size(float scroll_size);
  INLINE float get_scroll_size() const;

  INLINE void set_page_size(float page_size);
  INLINE float get_page_size() const;

  INLINE void set_value(float value);
  INLINE float get_value() const;

  INLINE void set_ratio(float ratio);
  INLINE float get_ratio() const;

  INLINE bool is_button_down() const;

  INLINE void set_resize_thumb(bool resize_thumb);
  INLINE bool get_resize_thumb() const;

  INLINE void set_manage_pieces(bool manage_pieces);
  INLINE bool get_manage_pieces() const;

  INLINE void set_thumb_button(PGButton *thumb_button);
  INLINE void clear_thumb_button();
  INLINE PGButton *get_thumb_button() const;

  INLINE void set_left_button(PGButton *left_button);
  INLINE void clear_left_button();
  INLINE PGButton *get_left_button() const;

  INLINE void set_right_button(PGButton *right_button);
  INLINE void clear_right_button();
  INLINE PGButton *get_right_button() const;

  INLINE static string get_adjust_prefix();
  INLINE string get_adjust_event() const;

  virtual void set_active(bool active);

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
  INLINE void internal_set_ratio(float ratio);

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

  float _min_value, _max_value;
  float _scroll_value, _scroll_ratio;
  float _page_value, _page_ratio;
  float _ratio;
  bool _resize_thumb;
  bool _manage_pieces;

  LVector3f _axis;

  PT(PGButton) _thumb_button;
  PT(PGButton) _left_button;
  PT(PGButton) _right_button;

  float _min_x, _max_x, _thumb_width, _range_x;
  LPoint3f _thumb_start;
  PGItem *_scroll_button_held;
  bool _mouse_button_page;
  LPoint2f _mouse_pos;
  double _next_advance_time;
  bool _dragging;
  float _drag_start_x;

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
