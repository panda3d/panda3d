// Filename: guiRollover.I
// Created by:  cary (26Oct00)
// 
////////////////////////////////////////////////////////////////////

INLINE GuiRollover::GuiRollover(void) {
}

INLINE void GuiRollover::enter(void) {
  if (_state) {
    if (gui_cat->is_debug())
      gui_cat->debug() << "entered '" << *this
		       << "' more then once without exit" << endl;
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
    if (gui_cat->is_debug())
      gui_cat->debug() << "exited '" << *this
		       << "' more then once without enter" << endl;
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
