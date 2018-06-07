/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pgWaitBar.h
 * @author drose
 * @date 2002-03-14
 */

#ifndef PGWAITBAR_H
#define PGWAITBAR_H

#include "pandabase.h"

#include "pgItem.h"

/**
 * This is a particular kind of PGItem that draws a little bar that fills from
 * left to right to indicate a slow process gradually completing, like a
 * traditional "wait, loading" bar.
 */
class EXPCL_PANDA_PGUI PGWaitBar : public PGItem {
PUBLISHED:
  explicit PGWaitBar(const std::string &name = "");
  virtual ~PGWaitBar();

protected:
  PGWaitBar(const PGWaitBar &copy);

public:
  virtual PandaNode *make_copy() const;
  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data);

PUBLISHED:
  void setup(PN_stdfloat width, PN_stdfloat height, PN_stdfloat range);

  INLINE void set_range(PN_stdfloat range);
  INLINE PN_stdfloat get_range() const;

  INLINE void set_value(PN_stdfloat value);
  INLINE PN_stdfloat get_value() const;

  INLINE PN_stdfloat get_percent() const;

  INLINE void set_bar_style(const PGFrameStyle &style);
  INLINE PGFrameStyle get_bar_style() const;

private:
  void update();

  PN_stdfloat _range, _value;
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
