// Filename: guiButton.cxx
// Created by:  cary (30Oct00)
// 
////////////////////////////////////////////////////////////////////

#include "guiButton.h"
#include "config_gui.h"

#include <throw_event.h>

#include <map>

typedef map<string, GuiButton*> ButtonMap;
static ButtonMap buttons;

TypeHandle GuiButton::_type_handle;


static GuiButton *
find_in_buttons_map(const string &name) {
  ButtonMap::const_iterator bi;
  bi = buttons.find(name);
  if (bi == buttons.end()) {
    return (GuiButton *)NULL;
  }
  return (*bi).second;
}

inline void GetExtents(GuiLabel* v, GuiLabel* w, GuiLabel* x, GuiLabel* y,
		       GuiLabel* z, float& l, float& r, float& b, float& t) {
  float l1, l2, r1, r2, b1, b2, t1, t2;
  v->get_extents(l1, r1, b1, t1);
  w->get_extents(l2, r2, b2, t2);
  l1 = (l1<l2)?l1:l2;
  r1 = (r1<r2)?r2:r1;
  b1 = (b1<b2)?b1:b2;
  t1 = (t1<t2)?t2:t1;
  if (x != (GuiLabel*)0L) {
    x->get_extents(l2, r2, b2, t2);
    l1 = (l1<l2)?l1:l2;
    r1 = (r1<r2)?r2:r1;
    b1 = (b1<b2)?b1:b2;
    t1 = (t1<t2)?t2:t1;
  }
  if (y != (GuiLabel*)0L) {
    y->get_extents(l2, r2, b2, t2);
    l1 = (l1<l2)?l1:l2;
    r1 = (r1<r2)?r2:r1;
    b1 = (b1<b2)?b1:b2;
    t1 = (t1<t2)?t2:t1;
  }
  if (z != (GuiLabel*)0L) {
    z->get_extents(l2, r2, b2, t2);
    l = (l1<l2)?l1:l2;
    r = (r1<r2)?r2:r1;
    b = (b1<b2)?b1:b2;
    t = (t1<t2)?t2:t1;
  }
}

static void enter_button(CPT_Event e) {
  GuiButton* val = find_in_buttons_map(e->get_name());
  if (val == (GuiButton *)NULL) {
    if (gui_cat.is_debug()) {
      gui_cat.debug()
	<< "Ignoring event " << e->get_name() << " for deleted button\n";
    }
    return;
  }
  val->test_ref_count_integrity();
  val->enter();
}

static void exit_button(CPT_Event e) {
  GuiButton* val = find_in_buttons_map(e->get_name());
  if (val == (GuiButton *)NULL) {
    if (gui_cat.is_debug()) {
      gui_cat.debug()
	<< "Ignoring event " << e->get_name() << " for deleted button\n";
    }
    return;
  }
  val->test_ref_count_integrity();
  val->exit();
}

static void click_button_down(CPT_Event e) {
  GuiButton* val = find_in_buttons_map(e->get_name());
  if (val == (GuiButton *)NULL) {
    if (gui_cat.is_debug()) {
      gui_cat.debug()
	<< "Ignoring event " << e->get_name() << " for deleted button\n";
    }
    return;
  }
  val->test_ref_count_integrity();
  val->down();
}

static void click_button_up(CPT_Event e) {
  GuiButton* val = find_in_buttons_map(e->get_name());
  if (val == (GuiButton *)NULL) {
    if (gui_cat.is_debug()) {
      gui_cat.debug()
	<< "Ignoring event " << e->get_name() << " for deleted button\n";
    }
    return;
  }
  val->test_ref_count_integrity();
  val->up();
}

void GuiButton::switch_state(GuiButton::States nstate) {
  if (_mgr == (GuiManager*)0L) {
    /*
    gui_cat.warning()
      << "Tried to switch state of unmanaged button\n";
    */
    _state = nstate;
    return;
  }

  test_ref_count_integrity();
  States ostate = _state;
  // cleanup old state
  switch (_state) {
  case NONE:
    break;
  case UP:
    _mgr->remove_label(_up);
    break;
  case UP_ROLLOVER:
    _mgr->remove_label(_up_rollover);
    break;
  case DOWN:
    _mgr->remove_label(_down);
    break;
  case DOWN_ROLLOVER:
    _mgr->remove_label(_down_rollover);
    break;
  case INACTIVE:
    if (_inactive != (GuiLabel*)0L)
      _mgr->remove_label(_inactive);
    break;
  case INACTIVE_ROLLOVER:
    if (_inactive != (GuiLabel*)0L)
      _mgr->remove_label(_inactive);
    break;
  default:
    gui_cat->warning() << "switching away from invalid state (" << (int)_state
		       << ")" << endl;
  }
  _state = nstate;
  // deal with new state
  switch (_state) {
  case NONE:
    _rgn->trap_clicks(false);
    break;
  case UP:
    if (_alt_root.is_null())
      _mgr->add_label(_up);
    else
      _mgr->add_label(_up, _alt_root);
    if (!_up_event.empty()) {
      gui_cat->debug() << "throwing _up_event '" << _up_event << "'" << endl;
      throw_event(_up_event);
    } else
      gui_cat->debug() << "_up_event is empty!" << endl;
    _rgn->trap_clicks(true);
    if ((ostate == INACTIVE) || (ostate == INACTIVE_ROLLOVER))
      _mgr->add_region(_rgn);
    break;
  case UP_ROLLOVER:
    if (_up_rollover != (GuiLabel*)0L) {
      if (_alt_root.is_null())
	_mgr->add_label(_up_rollover);
      else
	_mgr->add_label(_up_rollover, _alt_root);
      if (!_up_rollover_event.empty()) {
	gui_cat->debug() << "throwing _up_rollover_event '"
			 << _up_rollover_event << "'" << endl;
	throw_event(_up_rollover_event);
      } else
	gui_cat->debug() << "_up_rollover_event is empty!" << endl;
    } else {
      if (_alt_root.is_null())
	_mgr->add_label(_up);
      else
	_mgr->add_label(_up, _alt_root);
      if (!_up_event.empty()) {
	gui_cat->debug() << "throwing _up_event '" << _up_event << "'" << endl;
	throw_event(_up_event);
      } else
	gui_cat->debug() << "_up_event is empty!" << endl;
      _state = UP;
    }
    _rgn->trap_clicks(true);
    if ((ostate == INACTIVE) || (ostate == INACTIVE_ROLLOVER))
      _mgr->add_region(_rgn);
    break;
  case DOWN:
    if (_alt_root.is_null())
      _mgr->add_label(_down);
    else
      _mgr->add_label(_down, _alt_root);
    if (!_down_event.empty())
      throw_event(_down_event);
    else
      gui_cat->debug() << "_down_event is empty!" << endl;
    _rgn->trap_clicks(true);
    if ((ostate == INACTIVE) || (ostate == INACTIVE_ROLLOVER))
      _mgr->add_region(_rgn);
    break;
  case DOWN_ROLLOVER:
    if (_down_rollover != (GuiLabel*)0L) {
      if (_alt_root.is_null())
	_mgr->add_label(_down_rollover);
      else
	_mgr->add_label(_down_rollover, _alt_root);
      if (!_down_rollover_event.empty())
	throw_event(_down_rollover_event);
      else
	gui_cat->debug() << "_down_rollover_event is empty!" << endl;
    } else {
      if (_alt_root.is_null())
	_mgr->add_label(_down);
      else
	_mgr->add_label(_down, _alt_root);
      if (!_down_event.empty())
	throw_event(_down_event);
      else
	gui_cat->debug() << "_down_event is empty!" << endl;
      _state = DOWN;
    }
    _rgn->trap_clicks(true);
    if ((ostate == INACTIVE) || (ostate == INACTIVE_ROLLOVER))
      _mgr->add_region(_rgn);
    break;
  case INACTIVE:
    if (_inactive != (GuiLabel*)0L) {
      if (_alt_root.is_null())
	_mgr->add_label(_inactive);
      else
	_mgr->add_label(_inactive, _alt_root);
      if (!_inactive_event.empty())
	throw_event(_inactive_event);
    }
    _rgn->trap_clicks(false);
    if ((ostate != INACTIVE) && (ostate != INACTIVE_ROLLOVER))
      _mgr->remove_region(_rgn);
    break;
  case INACTIVE_ROLLOVER:
    if (_inactive != (GuiLabel*)0L) {
      if (_alt_root.is_null())
	_mgr->add_label(_inactive);
      else
	_mgr->add_label(_inactive, _alt_root);
      if (!_inactive_event.empty())
	throw_event(_inactive_event);
    }
    _rgn->trap_clicks(false);
    if ((ostate != INACTIVE) && (ostate != INACTIVE_ROLLOVER))
      _mgr->remove_region(_rgn);
    break;
  default:
    gui_cat->warning() << "switched to invalid state (" << (int)_state << ")"
		       << endl;
  }
  _mgr->recompute_priorities();
}

void GuiButton::recompute_frame(void) {
  GuiBehavior::recompute_frame();
  _up->recompute();
  _down->recompute();
  if (_up_rollover != (GuiLabel*)0L)
    _up_rollover->recompute();
  if (_down_rollover != (GuiLabel*)0L)
    _down_rollover->recompute();
  if (_inactive != (GuiLabel*)0L)
    _inactive->recompute();
  this->adjust_region();
}

void GuiButton::adjust_region(void) {
  GetExtents(_up, _down, _up_rollover, _down_rollover, _inactive, _left,
	     _right, _bottom, _top);
  GuiBehavior::adjust_region();
  _rgn->set_region(_left, _right, _bottom, _top);
}

void GuiButton::set_priority(GuiLabel* l, GuiItem::Priority p) {
  _up->set_priority(l, ((p==P_Low)?GuiLabel::P_LOWER:GuiLabel::P_HIGHER));
  _down->set_priority(l, ((p==P_Low)?GuiLabel::P_LOWER:GuiLabel::P_HIGHER));
  if (_up_rollover != (GuiLabel*)0L)
    _up_rollover->set_priority(l, ((p==P_Low)?GuiLabel::P_LOWER:
				   GuiLabel::P_HIGHER));
  if (_down_rollover != (GuiLabel*)0L)
    _down_rollover->set_priority(l, ((p==P_Low)?GuiLabel::P_LOWER:
				     GuiLabel::P_HIGHER));
  if (_inactive != (GuiLabel*)0L)
    _inactive->set_priority(l, ((p==P_Low)?GuiLabel::P_LOWER:
				GuiLabel::P_HIGHER));
  GuiItem::set_priority(l, p);
}

void GuiButton::behavior_up(CPT_Event, void* data) {
  GuiButton* button = (GuiButton*)data;
  gui_cat->debug() << "behavior_up (0x" << data << ")" << endl;
  button->run_button_up();
}

void GuiButton::behavior_down(CPT_Event, void* data) {
  GuiButton* button = (GuiButton*)data;
  gui_cat->debug() << "behavior_down (0x" << data << ")" << endl;
  button->run_button_down();
}

void GuiButton::run_button_up(void) {
  gui_cat->debug() << "run_button_up (0x" << (void*)this << " '" << this->get_name()
       << "')" << endl;
  if (_eh == (EventHandler*)0L)
    return;
  gui_cat->debug() << "doing work" << endl;
  _eh->remove_hook(_up_event, GuiButton::behavior_up, (void*)this);
  _eh->remove_hook(_up_rollover_event, GuiButton::behavior_up, (void*)this);
  if (!_behavior_event.empty()) {
    if (_have_event_param)
      throw_event(_behavior_event, EventParameter(_event_param));
    else
      throw_event(_behavior_event);
  }
  if (_behavior_functor != (GuiBehavior::BehaviorFunctor*)0L)
    _behavior_functor->doit(this);
}

void GuiButton::run_button_down(void) {
  gui_cat->debug() << "run_button_down (0x" << (void*)this << " '" << this->get_name()
       << "')" << endl;
  if (_eh == (EventHandler*)0L)
    return;
  gui_cat->debug() << "doing work, up_event is '" << _up_event << "' '"
       << _up_rollover_event << "'" << endl;
  _eh->add_hook(_up_event, GuiButton::behavior_up, (void*)this);
  _eh->add_hook(_up_rollover_event, GuiButton::behavior_up, (void*)this);
}

GuiButton::GuiButton(const string& name, GuiLabel* up, GuiLabel* down)
  : GuiBehavior(name), _up(up), _up_rollover((GuiLabel*)0L), _down(down),
    _down_rollover((GuiLabel*)0L), _inactive((GuiLabel*)0L),
    _up_event(name + "-up"), _up_rollover_event(""),
    _down_event(name +"-down"), _down_rollover_event(""),
    _inactive_event(""), _up_scale(up->get_scale()), _upr_scale(1.),
    _down_scale(down->get_scale()), _downr_scale(1.), _inactive_scale(1.),
    _state(GuiButton::NONE), _have_event_param(false), _event_param(0),
    _behavior_functor((GuiBehavior::BehaviorFunctor*)0L) {
  GetExtents(up, down, _up_rollover, _down_rollover, _inactive, _left, _right,
	     _bottom, _top);
  _rgn = new GuiRegion("button-" + name, _left, _right, _bottom, _top, true);
  buttons["gui-in-button-" + name] = this;
  buttons["gui-out-button-" + name] = this;
  buttons["gui-button-" + name + "-mouse1"] = this;
  buttons["gui-button-" + name + "-mouse2"] = this;
  buttons["gui-button-" + name + "-mouse3"] = this;
  buttons["gui-button-" + name + "-mouse1-up"] = this;
  buttons["gui-button-" + name + "-mouse2-up"] = this;
  buttons["gui-button-" + name + "-mouse3-up"] = this;
}

GuiButton::GuiButton(const string& name, GuiLabel* up, GuiLabel* down,
		     GuiLabel* inactive)
  : GuiBehavior(name), _up(up), _up_rollover((GuiLabel*)0L), _down(down),
    _down_rollover((GuiLabel*)0L), _inactive(inactive),
    _up_event(name + "-up"), _up_rollover_event(""),
    _down_event(name +"-down"), _down_rollover_event(""),
    _inactive_event(name + "-inactive"), _up_scale(up->get_scale()),
    _upr_scale(1.), _down_scale(down->get_scale()), _downr_scale(1.),
    _inactive_scale(inactive->get_scale()), _state(GuiButton::NONE),
    _have_event_param(false), _event_param(0),
    _behavior_functor((GuiBehavior::BehaviorFunctor*)0L) {
  GetExtents(up, down, _up_rollover, _down_rollover, inactive, _left, _right,
	     _bottom, _top);
  _rgn = new GuiRegion("button-" + name, _left, _right, _bottom, _top, true);
  buttons["gui-in-button-" + name] = this;
  buttons["gui-out-button-" + name] = this;
  buttons["gui-button-" + name + "-mouse1"] = this;
  buttons["gui-button-" + name + "-mouse2"] = this;
  buttons["gui-button-" + name + "-mouse3"] = this;
  buttons["gui-button-" + name + "-mouse1-up"] = this;
  buttons["gui-button-" + name + "-mouse2-up"] = this;
  buttons["gui-button-" + name + "-mouse3-up"] = this;
}

GuiButton::GuiButton(const string& name, GuiLabel* up, GuiLabel* up_roll,
		     GuiLabel* down, GuiLabel* down_roll, GuiLabel* inactive)
  : GuiBehavior(name), _up(up), _up_rollover(up_roll), _down(down),
    _down_rollover(down_roll), _inactive(inactive), _up_event(name + "-up"),
    _up_rollover_event(name + "-up-rollover"), _down_event(name +"-down"),
    _down_rollover_event(name + "-down-rollover"),
    _inactive_event(name + "-inactive"), _up_scale(up->get_scale()),
    _upr_scale(up_roll->get_scale()), _down_scale(down->get_scale()),
    _downr_scale(down_roll->get_scale()),
    _inactive_scale(inactive->get_scale()), _state(GuiButton::NONE),
    _have_event_param(false), _event_param(0),
    _behavior_functor((GuiBehavior::BehaviorFunctor*)0L) {
  GetExtents(up, down, up_roll, down_roll, inactive, _left, _right, _bottom,
	     _top);
  _rgn = new GuiRegion("button-" + name, _left, _right, _bottom, _top, true);
  buttons["gui-in-button-" + name] = this;
  buttons["gui-out-button-" + name] = this;
  buttons["gui-button-" + name + "-mouse1"] = this;
  buttons["gui-button-" + name + "-mouse2"] = this;
  buttons["gui-button-" + name + "-mouse3"] = this;
  buttons["gui-button-" + name + "-mouse1-up"] = this;
  buttons["gui-button-" + name + "-mouse2-up"] = this;
  buttons["gui-button-" + name + "-mouse3-up"] = this;
}

GuiButton::~GuiButton(void) {
  this->unmanage();

  // Remove the names from the buttons map, so we don't end up with
  // an invalid pointer.
  string name = get_name();
  buttons.erase("gui-in-button-" + name);
  buttons.erase("gui-out-button-" + name);
  buttons.erase("gui-button-" + name + "-mouse1");
  buttons.erase("gui-button-" + name + "-mouse2");
  buttons.erase("gui-button-" + name + "-mouse3");
  buttons.erase("gui-button-" + name + "-mouse1-up");
  buttons.erase("gui-button-" + name + "-mouse2-up");
  buttons.erase("gui-button-" + name + "-mouse3-up");

  if (_behavior_functor != (GuiBehavior::BehaviorFunctor*)0L)
    delete _behavior_functor;
}

void GuiButton::manage(GuiManager* mgr, EventHandler& eh) {
  if (!_added_hooks) {
    eh.add_hook("gui-in-button-" + get_name(), enter_button);
    eh.add_hook("gui-out-button-" + get_name(), exit_button);
    eh.add_hook("gui-button-" + get_name() + "-mouse1", click_button_down);
    eh.add_hook("gui-button-" + get_name() + "-mouse2", click_button_down);
    eh.add_hook("gui-button-" + get_name() + "-mouse3", click_button_down);
    eh.add_hook("gui-button-" + get_name() + "-mouse1-up", click_button_up);
    eh.add_hook("gui-button-" + get_name() + "-mouse2-up", click_button_up);
    eh.add_hook("gui-button-" + get_name() + "-mouse3-up", click_button_up);
    _added_hooks = true;
  }
  if (_mgr == (GuiManager*)0L) {
    mgr->add_region(_rgn);
    GuiBehavior::manage(mgr, eh);
    if (_behavior_running)
      this->start_behavior();
    switch_state(UP);
  } else
    gui_cat->warning() << "tried to manage button (0x" << (void*)this
		       << ") that is already managed" << endl;
}

void GuiButton::manage(GuiManager* mgr, EventHandler& eh, Node* n) {
  if (!_added_hooks) {
    eh.add_hook("gui-in-button-" + get_name(), enter_button);
    eh.add_hook("gui-out-button-" + get_name(), exit_button);
    eh.add_hook("gui-button-" + get_name() + "-mouse1", click_button_down);
    eh.add_hook("gui-button-" + get_name() + "-mouse2", click_button_down);
    eh.add_hook("gui-button-" + get_name() + "-mouse3", click_button_down);
    eh.add_hook("gui-button-" + get_name() + "-mouse1-up", click_button_up);
    eh.add_hook("gui-button-" + get_name() + "-mouse2-up", click_button_up);
    eh.add_hook("gui-button-" + get_name() + "-mouse3-up", click_button_up);
    _added_hooks = true;
  }
  if (_mgr == (GuiManager*)0L) {
    mgr->add_region(_rgn);
    GuiBehavior::manage(mgr, eh, n);
    if (_behavior_running)
      this->start_behavior();
    switch_state(UP);
  } else
    gui_cat->warning() << "tried to manage button (0x" << (void*)this
		       << ") that is already managed" << endl;
}

void GuiButton::unmanage(void) {
  if (_mgr != (GuiManager*)0L)
    if ((_state != NONE) && (_state != INACTIVE) &&
	(_state != INACTIVE_ROLLOVER))
      _mgr->remove_region(_rgn);
  if (_behavior_running)
    this->stop_behavior();
  switch_state(NONE);
  GuiBehavior::unmanage();
}

int GuiButton::freeze() {
  _up->freeze();
  _down->freeze();
  if (_up_rollover != (GuiLabel*)0L)
    _up_rollover->freeze();
  if (_down_rollover != (GuiLabel*)0L)
    _down_rollover->freeze();
  if (_inactive != (GuiLabel*)0L)
    _inactive->freeze();

  return 0;
}

int GuiButton::thaw() {
  _up->thaw();
  _down->thaw();
  if (_up_rollover != (GuiLabel*)0L)
    _up_rollover->thaw();
  if (_down_rollover != (GuiLabel*)0L)
    _down_rollover->thaw();
  if (_inactive != (GuiLabel*)0L)
    _inactive->thaw();

  return 0;
}

void GuiButton::set_scale(float f) {
  _up->set_scale(f * _up_scale);
  _down->set_scale(f * _down_scale);
  if (_up_rollover != (GuiLabel*)0L)
    _up_rollover->set_scale(f * _upr_scale);
  if (_down_rollover != (GuiLabel*)0L)
    _down_rollover->set_scale(f * _downr_scale);
  if (_inactive != (GuiLabel*)0L)
    _inactive->set_scale(f * _inactive_scale);
  GuiBehavior::set_scale(f);
  this->recompute_frame();
}

void GuiButton::set_scale(float x, float y, float z) {
  _up->set_scale(x, y, z);
  _down->set_scale(x, y, z);
  if (_up_rollover != (GuiLabel*)0L)
    _up_rollover->set_scale(x, y, z);
  if (_down_rollover != (GuiLabel*)0L)
    _down_rollover->set_scale(x, y, z);
  if (_inactive != (GuiLabel*)0L)
    _inactive->set_scale(x, y, z);
  GuiBehavior::set_scale(x, y, z);
  this->recompute_frame();
}

void GuiButton::set_pos(const LVector3f& p) {
  _up->set_pos(p);
  _down->set_pos(p);
  if (_up_rollover != (GuiLabel*)0L)
    _up_rollover->set_pos(p);
  if (_down_rollover != (GuiLabel*)0L)
    _down_rollover->set_pos(p);
  if (_inactive != (GuiLabel*)0L)
    _inactive->set_pos(p);
  GuiBehavior::set_pos(p);
  this->recompute_frame();
}

void GuiButton::start_behavior(void) {
  GuiBehavior::start_behavior();
  if (_mgr == (GuiManager*)0L)
    return;
  if (!this->is_active())
    return;
  _eh->add_hook(_down_event, GuiButton::behavior_down, (void*)this);
  _eh->add_hook(_down_rollover_event, GuiButton::behavior_down, (void*)this);
}

void GuiButton::stop_behavior(void) {
  GuiBehavior::stop_behavior();
  if (_mgr == (GuiManager*)0L)
    return;
  _eh->remove_hook(_up_event, GuiButton::behavior_up, (void*)this);
  _eh->remove_hook(_up_rollover_event, GuiButton::behavior_up, (void*)this);
  _eh->remove_hook(_down_event, GuiButton::behavior_down, (void*)this);
  _eh->remove_hook(_down_rollover_event, GuiButton::behavior_down,
		   (void*)this);
}

void GuiButton::reset_behavior(void) {
  gui_cat->debug() << this->get_name() << "::reset_behavior()" << endl;
  GuiBehavior::reset_behavior();
  if (_mgr == (GuiManager*)0L)
    return;
  this->start_behavior();
  _eh->remove_hook(_up_event, GuiButton::behavior_up, (void*)this);
  _eh->remove_hook(_up_rollover_event, GuiButton::behavior_up, (void*)this);
}

void GuiButton::set_priority(GuiItem* i, const GuiItem::Priority p) {
  if (p == P_Highest) {
    _up->set_priority(_up, GuiLabel::P_HIGHEST);
    _down->set_priority(_up, GuiLabel::P_HIGHEST);
    if (_up_rollover != (GuiLabel*)0L)
      _up_rollover->set_priority(_up, GuiLabel::P_HIGHEST);
    if (_down_rollover != (GuiLabel*)0L)
      _down_rollover->set_priority(_up, GuiLabel::P_HIGHEST);
    if (_inactive != (GuiLabel*)0L)
      _inactive->set_priority(_up, GuiLabel::P_HIGHEST);
  } else if (p == P_Lowest) {
    _up->set_priority(_up, GuiLabel::P_LOWEST);
    _down->set_priority(_up, GuiLabel::P_LOWEST);
    if (_up_rollover != (GuiLabel*)0L)
      _up_rollover->set_priority(_up, GuiLabel::P_LOWEST);
    if (_down_rollover != (GuiLabel*)0L)
      _down_rollover->set_priority(_up, GuiLabel::P_LOWEST);
    if (_inactive != (GuiLabel*)0L)
      _inactive->set_priority(_up, GuiLabel::P_LOWEST);
  } else {
    i->set_priority(_up, ((p==P_Low)?P_High:P_Low));
    i->set_priority(_down, ((p==P_Low)?P_High:P_Low));
    if (_up_rollover != (GuiLabel*)0L)
      i->set_priority(_up_rollover, ((p==P_Low)?P_High:P_Low));
    if (_down_rollover != (GuiLabel*)0L)
      i->set_priority(_down_rollover, ((p==P_Low)?P_High:P_Low));
    if (_inactive != (GuiLabel*)0L)
      i->set_priority(_inactive, ((p==P_Low)?P_High:P_Low));
  }
  GuiBehavior::set_priority(i, p);
}

int GuiButton::set_draw_order(int v) {
  // No two of these labels will ever be drawn simultaneously, so
  // there's no need to cascade the draw orders.  They can each be
  // assigned the same value, and the value we return is the maximum
  // of any of the values returned by the labels.
  int o = _rgn->set_draw_order(v);
  int o1 = _up->set_draw_order(v);
  o = max(o, o1);
  o1 = _down->set_draw_order(v);
  o = max(o, o1);
  if (_up_rollover != (GuiLabel*)0L) {
    o1 = _up_rollover->set_draw_order(v);
    o = max(o, o1);
  }
  if (_down_rollover != (GuiLabel*)0L) {
    o1 = _down_rollover->set_draw_order(v);
    o = max(o, o1);
  }
  if (_inactive != (GuiLabel*)0L) {
    o1 = _inactive->set_draw_order(v);
    o = max(o, o1);
  }
  o1 = GuiBehavior::set_draw_order(v);
  o = max(o, o1);
  return o;
}

void GuiButton::output(ostream& os) const {
  GuiBehavior::output(os);
  os << "  Button data:" << endl;
  os << "    up - 0x" << (void*)_up << endl;
  os << "    up_rollover - 0x" << (void*)_up_rollover << endl;
  os << "    down - 0x" << (void*)_down << endl;
  os << "    down_rollover - 0x" << (void*)_down_rollover << endl;
  os << "    inactive - 0x" << (void*)_inactive << endl;
  os << "    up event - '" << _up_event << "'" << endl;
  os << "    up_rollover event - '" << _up_rollover_event << "'" << endl;
  os << "    down event - '" << _down_event << "'" << endl;
  os << "    down_rollover event - '" << _down_rollover_event << "'" << endl;
  os << "    inactive event - '" << _inactive_event << "'" << endl;
  os << "    rgn - 0x" << (void*)_rgn << endl;
  os << "      frame - " << _rgn->get_frame() << endl;
  os << "    state - " << (int)_state << endl;
}
