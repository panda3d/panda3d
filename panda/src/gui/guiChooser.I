// Filename: guiChooser.I
// Created by:  cary (08Feb01)
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

INLINE GuiChooser::GuiChooser(void) {}

INLINE int GuiChooser::get_curr_item(void) const {
  return _curr;
}

INLINE void GuiChooser::set_loop(bool b) {
  _loop = b;
}

INLINE bool GuiChooser::get_loop(void) const {
  return _loop;
}
