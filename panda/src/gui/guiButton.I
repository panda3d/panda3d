// Filename: guiButton.I
// Created by:  cary (30Oct00)
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

INLINE GuiButton::GuiButton(void) {
}

INLINE bool GuiButton::owns_region(const MouseWatcherRegion* rgn) const {
  return (_rgn == rgn);
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
  case NONE:
  case DOWN:
  case INACTIVE:
    switch_state(UP);
    break;
  case DOWN_ROLLOVER:
  case INACTIVE_ROLLOVER:
    switch_state(UP_ROLLOVER);
    break;
  case UP:
  case UP_ROLLOVER:
    break;
  default:
    gui_cat->warning() << "got up from invalid state (" << (int)_state << "),"
                       << " button '" << this->get_name() << "'" << endl;
  }
}

INLINE void GuiButton::down(void) {
  switch (_state) {
  case NONE:
  case UP:
  case INACTIVE:
    switch_state(DOWN);
    break;
  case UP_ROLLOVER:
  case INACTIVE_ROLLOVER:
    switch_state(DOWN_ROLLOVER);
    break;
  case DOWN:
  case DOWN_ROLLOVER:
    break;
  default:
    gui_cat->warning() << "got down from invalid state (" << (int)_state
                       << "), button '" << this->get_name() << "'" << endl;
  }
}

INLINE void GuiButton::inactive(void) {
  switch (_state) {
  case NONE:
  case UP:
  case DOWN:
    switch_state(INACTIVE);
    break;
  case UP_ROLLOVER:
  case DOWN_ROLLOVER:
    switch_state(INACTIVE_ROLLOVER);
    break;
  case INACTIVE:
  case INACTIVE_ROLLOVER:
    break;
  default:
    gui_cat->warning() << "got inactive from invalid state (" << (int)_state
                       << "), button '" << this->get_name() << "'" << endl;
  }
  this->stop_behavior();
}

INLINE void GuiButton::click(void) {
  switch (_state) {
  case NONE:
  case UP:
  case UP_ROLLOVER:
    down();
    break;
  case DOWN:
  case DOWN_ROLLOVER:
    up();
    break;
  case INACTIVE:
  case INACTIVE_ROLLOVER:
    break;
  default:
    gui_cat->warning() << "got click from invalid state (" << (int)_state
                       << "), button '" << this->get_name() << "'" << endl;
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
  if (_behavior_running)
    if (_mgr != (GuiManager*)0L)
      _eh->remove_hook(_up_event, GuiButton::behavior_up);
  _up_event = s;
  if (_behavior_running)
    if (_mgr != (GuiManager*)0L)
      _eh->add_hook(_up_event, GuiButton::behavior_up);
}

INLINE void GuiButton::set_up_rollover_event(const string& s) {
  if (_behavior_running)
    if (_mgr != (GuiManager*)0L)
      _eh->remove_hook(_up_rollover_event, GuiButton::behavior_up);
  _up_rollover_event = s;
  if (_behavior_running)
    if (_mgr != (GuiManager*)0L)
      _eh->add_hook(_up_rollover_event, GuiButton::behavior_up);
}

INLINE void GuiButton::set_down_event(const string& s) {
  if (_behavior_running)
    if (_mgr != (GuiManager*)0L)
      _eh->remove_hook(_down_event, GuiButton::behavior_up);
  _down_event = s;
  if (_behavior_running)
    if (_mgr != (GuiManager*)0L)
      _eh->add_hook(_down_event, GuiButton::behavior_up);
}

INLINE void GuiButton::set_down_rollover_event(const string& s) {
  if (_behavior_running)
    if (_mgr != (GuiManager*)0L)
      _eh->remove_hook(_down_rollover_event, GuiButton::behavior_up);
  _down_rollover_event = s;
  if (_behavior_running)
    if (_mgr != (GuiManager*)0L)
      _eh->add_hook(_down_rollover_event, GuiButton::behavior_up);
}

INLINE void GuiButton::set_inactive_event(const string& s) {
  _inactive_event = s;
}

INLINE void GuiButton::set_behavior_event(const string& s) {
  _behavior_event = s;
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

INLINE const string& GuiButton::get_behavior_event(void) const {
  return _behavior_event;
}

INLINE void GuiButton::set_up_rollover(GuiLabel* upr) {
  _up_rollover = upr;
  if (_up_rollover_event.empty())
    _up_rollover_event = this->get_name() + "-up-rollover";
}

INLINE void GuiButton::set_down_rollover(GuiLabel* downr) {
  _down_rollover = downr;
  if (_down_rollover_event.empty())
    _down_rollover_event = this->get_name() + "-down-rollover";
}

INLINE void GuiButton::set_behavior_functor(GuiBehavior::BehaviorFunctor* f) {
  _behavior_functor = f;
}

INLINE GuiBehavior::BehaviorFunctor*
GuiButton::get_behavior_functor(void) const {
  return _behavior_functor;
}

INLINE void GuiButton::set_rollover_functor(GuiBehavior::BehaviorFunctor* f) {
  _rollover_functor = f;
}

INLINE GuiBehavior::BehaviorFunctor*
GuiButton::get_rollover_functor(void) const {
  return _rollover_functor;
}

INLINE void GuiButton::set_behavior_event_parameter(int p) {
  this->_have_event_param = true;
  this->_event_param = p;
}

INLINE int GuiButton::get_behavior_event_parameter(void) const {
  return this->_event_param;
}

INLINE void GuiButton::set_up_label(GuiLabel* l) {
  _up = l;
}

INLINE void GuiButton::set_up_rollover_label(GuiLabel* l) {
  _up_rollover = l;
}

INLINE void GuiButton::set_down_label(GuiLabel* l) {
  _down = l;
}

INLINE void GuiButton::set_down_rollover_label(GuiLabel* l) {
  _down_rollover = l;
}

INLINE void GuiButton::set_inactive_label(GuiLabel* l){
  _inactive = l;
}

INLINE GuiLabel* GuiButton::get_up_label(void) const {
  return _up;
}

INLINE GuiLabel* GuiButton::get_up_rollover_label(void) const {
  return _up_rollover;
}

INLINE GuiLabel* GuiButton::get_down_label(void) const {
  return _down;
}

INLINE GuiLabel* GuiButton::get_down_rollover_label(void) const {
  return _down_rollover;
}

INLINE GuiLabel* GuiButton::get_inactive_label(void) const {
  return _inactive;
}
