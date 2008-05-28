// Filename: pgWaitBar.h
// Created by:  drose (14Mar02)
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

#ifndef PGWAITBAR_H
#define PGWAITBAR_H

#include "pandabase.h"

#include "pgItem.h"

////////////////////////////////////////////////////////////////////
//       Class : PGWaitBar
// Description : This is a particular kind of PGItem that draws a
//               little bar that fills from left to right to indicate
//               a slow process gradually completing, like a
//               traditional "wait, loading" bar.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PGUI PGWaitBar : public PGItem {
PUBLISHED:
  PGWaitBar(const string &name = "");
  virtual ~PGWaitBar();

protected:
  PGWaitBar(const PGWaitBar &copy);

public:
  virtual PandaNode *make_copy() const;
  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data);

PUBLISHED:
  void setup(float width, float height, float range);

  INLINE void set_range(float range);
  INLINE float get_range() const;

  INLINE void set_value(float value);
  INLINE float get_value() const;

  INLINE float get_percent() const;

  INLINE void set_bar_style(const PGFrameStyle &style);
  INLINE PGFrameStyle get_bar_style() const;

private:
  void update();

  float _range, _value;
  int _bar_state;
  PGFrameStyle _bar_style;
  NodePath _bar;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PGItem::init_type();
    register_type(_type_handle, "PGWaitBar",
                  PGItem::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "pgWaitBar.I"

#endif
