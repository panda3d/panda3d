// Filename: qppgWaitBar.h
// Created by:  drose (14Mar02)
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

#ifndef qpPGWAITBAR_H
#define qpPGWAITBAR_H

#include "pandabase.h"

#include "qppgItem.h"

////////////////////////////////////////////////////////////////////
//       Class : qpPGWaitBar
// Description : This is a particular kind of qpPGItem that draws a
//               little bar that fills from left to right to indicate
//               a slow process gradually completing, like a
//               traditional "wait, loading" bar.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpPGWaitBar : public qpPGItem {
PUBLISHED:
  qpPGWaitBar(const string &name = "");
  virtual ~qpPGWaitBar();

protected:
  qpPGWaitBar(const qpPGWaitBar &copy);

public:
  virtual PandaNode *make_copy() const;
  virtual bool has_cull_callback() const;
  virtual bool cull_callback(qpCullTraverser *trav, CullTraverserData &data);

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
  PT(PandaNode) _bar_node;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    qpPGItem::init_type();
    register_type(_type_handle, "qpPGWaitBar",
                  qpPGItem::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "qppgWaitBar.I"

#endif
