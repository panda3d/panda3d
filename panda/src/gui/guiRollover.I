// Filename: guiRollover.I
// Created by:  cary (26Oct00)
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

INLINE GuiRollover::GuiRollover(void) {
}

INLINE void GuiRollover::enter(void) {
  if (_state) {
#ifdef _DEBUG
    if (gui_cat->is_debug())
      gui_cat->debug() << "entered '" << *this
               << "' more then once without exit" << endl;
#endif
  } else if (_mgr != (GuiManager*)0L) {
    _mgr->remove_label(_off);
    if (_alt_root.is_null())
      _mgr->add_label(_on);
    else
      _mgr->add_label(_on, _alt_root);
    _state = true;
  }
}

INLINE void GuiRollover::exit(void) {
  if (!_state) {
#ifdef _DEBUG
    if (gui_cat.is_debug())
      gui_cat->debug() << "exited '" << *this
               << "' more then once without enter" << endl;
#endif
  } else if (_mgr != (GuiManager*)0L) {
    _mgr->remove_label(_on);
    if (_alt_root.is_null())
      _mgr->add_label(_off);
    else
      _mgr->add_label(_off, _alt_root);
    _state = false;
  }
}

INLINE bool GuiRollover::is_over(void) const {
  return _state;
}

INLINE bool GuiRollover::owns_region(const MouseWatcherRegion* rgn) const {
  return (_rgn == rgn);
}
