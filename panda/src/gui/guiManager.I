// Filename: guiManager.I
// Created by:  cary (25Oct00)
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

INLINE GuiManager::GuiManager(MouseWatcher* w, Node* n, EventHandler* h)
  : _start_draw_order(0), _next_draw_order(0), _root(n), _watcher(w),
    _eh(h) {
}

INLINE int GuiManager::get_next_draw_order(void) const {
  return _next_draw_order;
}

INLINE void GuiManager::set_next_draw_order(int v) {
#ifndef NDEBUG
  if (v < _next_draw_order)
    gui_cat->warning() << "GuiManager::set_next_draw_order(" << v
                       << ") is < existing (" << _next_draw_order << ")"
                       << endl;
#endif /* NDEBUG */
  this->_next_draw_order = v;
}

INLINE int GuiManager::get_start_draw_order(void) const {
  return _start_draw_order;
}

INLINE void GuiManager::set_start_draw_order(int v) {
  this->_start_draw_order = v;
}

INLINE EventHandler* GuiManager::get_private_handler(void) const {
  return _eh;
}
