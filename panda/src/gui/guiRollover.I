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
    _mgr->add_label(_on);
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
    _mgr->add_label(_off);
    _state = false;
  }
}

INLINE bool GuiRollover::is_over(void) const {
  return _state;
}

INLINE void GuiRollover::set_scale(float f) {
  _on->set_scale(f);
  _off->set_scale(f);
  recompute_frame();
  _scale = f;
}

INLINE void GuiRollover::set_pos(const LVector3f& p) {
  _on->set_pos(p);
  _off->set_pos(p);
  recompute_frame();
  _pos = p;
}

INLINE float GuiRollover::get_scale(void) const {
  return _scale;
}

INLINE LVector3f GuiRollover::get_pos(void) const {
  return _pos;
}
