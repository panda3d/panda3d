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
#include "pgSliderButton.h"

////////////////////////////////////////////////////////////////////
//       Class : PGSliderBar
// Description : This is a particular kind of PGItem that draws a
//               little bar with a slider that moves from left to 
//               right indicating a value between the ranges
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PGSliderBar : public PGItem {
PUBLISHED:
  PGSliderBar(const string &name = "");
  virtual ~PGSliderBar();

protected:
  PGSliderBar(const PGSliderBar &copy);

public:
  virtual PandaNode *make_copy() const;
  virtual bool has_cull_callback() const;
  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data);

  virtual void press(const MouseWatcherParameter &param, bool background);
  virtual void drag(const MouseWatcherParameter &param);

PUBLISHED:
  void setup(float width, float height, float range);

  INLINE void set_range(float range);
  INLINE float get_range() const;

  INLINE void set_value(float value);
  INLINE float get_value() const;

  INLINE void set_speed(float speed);
  INLINE float get_speed() const;

  INLINE float get_percent() const;

  INLINE void set_bar_style(const PGFrameStyle &style);
  INLINE PGFrameStyle get_bar_style() const;

  INLINE NodePath get_slider_button() const;
  INLINE NodePath get_left_button() const;
  INLINE NodePath get_right_button() const;

private:
  void update();

  bool _update_slider;
  float _update_value;

  float _range, _value;
  float _speed, _width;
  int _bar_state;
  PGFrameStyle _bar_style;
  PGSliderButton _slider;
  PGSliderButton _left;
  PGSliderButton _right;
  NodePath _bar;
  NodePath _slider_button;
  NodePath _left_button;
  NodePath _right_button;

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
