// Filename: guiButton.I
// Created by:  cary (30Oct00)
// 
////////////////////////////////////////////////////////////////////

INLINE GuiButton::GuiButton(void) {
}

INLINE void GuiButton::enter(void) {
  switch (_state) {
  case UP:
    switch_state(UP_ROLLOVER);
    break;
  case DOWN:
    switch_state(DOWN_ROLLOVER);
    break;
  case INACTIVE:
    switch_state(INACTIVE_ROLLOVER);
    break;
  default:
    break;
  }
}

INLINE void GuiButton::exit(void) {
  switch (_state) {
  case UP_ROLLOVER:
    switch_state(UP);
    break;
  case DOWN_ROLLOVER:
    switch_state(DOWN);
    break;
  case INACTIVE_ROLLOVER:
    switch_state(INACTIVE);
    break;
  default:
    break;
  }
}

INLINE void GuiButton::up(void) {
  switch (_state) {
  case DOWN:
    switch_state(UP);
    break;
  case DOWN_ROLLOVER:
    switch_state(UP_ROLLOVER);
    break;
  default:
    gui_cat->warning() << "got up from invalid state (" << _state << ")"
		       << endl;
  }
}

INLINE void GuiButton::down(void) {
  switch (_state) {
  case UP:
    switch_state(DOWN);
    break;
  case UP_ROLLOVER:
    switch_state(DOWN_ROLLOVER);
    break;
  default:
    gui_cat->warning() << "got down from invalid state (" << _state << ")"
		       << endl;
  }
}

INLINE void GuiButton::inactive(void) {
  switch (_state) {
  case UP:
  case DOWN:
    switch_state(INACTIVE);
    break;
  case UP_ROLLOVER:
  case DOWN_ROLLOVER:
    switch_state(INACTIVE_ROLLOVER);
    break;
  default:
    gui_cat->warning() << "got inactive from invalid state (" << _state << ")"
		       << endl;
  }
}

INLINE void GuiButton::click(void) {
  switch (_state) {
  case UP:
  case UP_ROLLOVER:
    down();
    break;
  case DOWN:
  case DOWN_ROLLOVER:
    up();
    break;
  case INACTIVE:
    break;
  default:
    gui_cat->warning() << "got click from invalid state (" << _state << ")"
		       << endl;
  }
}

INLINE bool GuiButton::is_up(void) const {
  if ((_state == UP) || (_state == UP_ROLLOVER))
    return true;
  return false;
}

INLINE bool GuiButton::is_over(void) const {
  if ((_state == UP_ROLLOVER) || (_state == DOWN_ROLLOVER))
    return true;
  return false;
}

INLINE bool GuiButton::is_active(void) const {
  if ((_state == INACTIVE) || (_state == NONE))
    return false;
  return true;
}

INLINE void GuiButton::set_up_event(const string& s) {
  _up_event = s;
}

INLINE void GuiButton::set_up_rollover_event(const string& s) {
  _up_rollover_event = s;
}

INLINE void GuiButton::set_down_event(const string& s) {
  _down_event = s;
}

INLINE void GuiButton::set_down_rollover_event(const string& s) {
  _down_rollover_event = s;
}

INLINE void GuiButton::set_inactive_event(const string& s) {
  _inactive_event = s;
}

INLINE const string& GuiButton::get_up_event(void) const {
  return _up_event;
}

INLINE const string& GuiButton::get_up_rollover_event(void) const {
  return _up_rollover_event;
}

INLINE const string& GuiButton::get_down_event(void) const {
  return _down_event;
}

INLINE const string& GuiButton::get_down_rollover_event(void) const {
  return _down_rollover_event;
}

INLINE const string& GuiButton::get_inactive_event(void) const {
  return _inactive_event;
}

INLINE void GuiButton::set_scale(float f) {
  _up->set_scale(f);
  _down->set_scale(f);
  if (_up_rollover != (GuiLabel*)0L)
    _up_rollover->set_scale(f);
  if (_down_rollover != (GuiLabel*)0L)
    _down_rollover->set_scale(f);
  if (_inactive != (GuiLabel*)0L)
    _inactive->set_scale(f);
  _scale = f;
  recompute_frame();
}

INLINE void GuiButton::set_pos(const LVector3f& p) {
  _up->set_pos(p);
  _down->set_pos(p);
  if (_up_rollover != (GuiLabel*)0L)
    _up_rollover->set_pos(p);
  if (_down_rollover != (GuiLabel*)0L)
    _down_rollover->set_pos(p);
  if (_inactive != (GuiLabel*)0L)
    _inactive->set_pos(p);
  _pos = p;
  recompute_frame();
}

INLINE float GuiButton::get_scale(void) const {
  return _scale;
}

INLINE LVector3f GuiButton::get_pos(void) const {
  return _pos;
}
