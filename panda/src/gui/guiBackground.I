// Filename: guiBackground.I
// Created by:  cary (05Feb01)
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

INLINE GuiBackground::GuiBackground(void) {
}

INLINE void GuiBackground::set_color(float r, float g, float b, float a) {
  _bg->set_foreground_color(r, g, b, a);
}

INLINE void GuiBackground::set_color(const Colorf& c) {
  _bg->set_foreground_color(c);
}

INLINE Colorf GuiBackground::get_color(void) const {
  return _bg->get_foreground_color();
}

INLINE void GuiBackground::reassert(void) {
  _item->set_priority(_bg, P_High);
}
