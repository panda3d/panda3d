// Filename: guiItem.I
// Created by:  cary (01Nov00)
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

INLINE GuiItem::GuiItem(void) : Namable("fubar"), _scale(1.), _scale_x(1.),
                                _scale_y(1.), _scale_z(1.), _left(-1.),
                                _right(1.), _bottom(-1.), _top(1.),
                                _pri(P_Normal) {}

#include <get_rel_pos.h>

INLINE void GuiItem::get_graph_mat(LMatrix4f& m) {
  if (_alt_root.is_null()) {
    m = LMatrix4f::ident_mat();
    return;
  }
  // pass NULL for the 'to' to go to root
  get_rel_mat(_alt_root, (Node*)0L, m);
}

INLINE float GuiItem::get_scale(void) const {
  return _scale;
}

INLINE float GuiItem::get_scale_x(void) const {
  return _scale_x;
}

INLINE float GuiItem::get_scale_y(void) const {
  return _scale_y;
}

INLINE float GuiItem::get_scale_z(void) const {
  return _scale_z;
}

INLINE LVector3f GuiItem::get_pos(void) const {
  return _pos;
}

INLINE float GuiItem::get_left(void) const {
  return _left;
}

INLINE float GuiItem::get_right(void) const {
  return _right;
}

INLINE float GuiItem::get_bottom(void) const {
  return _bottom;
}

INLINE float GuiItem::get_top(void) const {
  return _top;
}

INLINE LVector4f GuiItem::get_frame(void) const {
  return LVector4f(_left, _right, _bottom, _top);
}

INLINE float GuiItem::get_width(void) const {
  return _right - _left;
}

INLINE float GuiItem::get_height(void) const {
  return _top - _bottom;
}

INLINE GuiItem::Priority GuiItem::get_priority(void) const {
  return _pri;
}

INLINE void GuiItem::recompute(void) {
  this->recompute_frame();
}

INLINE ostream& operator<<(ostream& os, GuiItem& item) {
  item.output(os);
  return os;
}
