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

static void click_button(CPT_Event e) {
  GuiButton* val = find_in_buttons_map(e->get_name());
  if (val == (GuiButton *)NULL) {
    if (gui_cat.is_debug()) {
      gui_cat.debug()
	<< "Ignoring event " << e->get_name() << " for deleted button\n";
    }
    return;
  }
  val->test_ref_count_integrity();
  val->click();
}

void GuiButton::switch_state(GuiButton::States nstate) {
  test_ref_count_integrity();
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
    _mgr->add_label(_up);
    if (!_up_event.empty())
      throw_event(_up_event);
    _rgn->trap_clicks(true);
    break;
  case UP_ROLLOVER:
    if (_up_rollover != (GuiLabel*)0L) {
      _mgr->add_label(_up_rollover);
      if (!_up_rollover_event.empty())
	throw_event(_up_rollover_event);
    } else {
      _mgr->add_label(_up);
      if (!_up_event.empty())
	throw_event(_up_event);
      _state = UP;
    }
    _rgn->trap_clicks(true);
    break;
  case DOWN:
    _mgr->add_label(_down);
    if (!_down_event.empty())
      throw_event(_down_event);
    _rgn->trap_clicks(true);
    break;
  case DOWN_ROLLOVER:
    if (_down_rollover != (GuiLabel*)0L) {
      _mgr->add_label(_down_rollover);
      if (!_down_rollover_event.empty())
	throw_event(_down_rollover_event);
    } else {
      _mgr->add_label(_down);
      if (!_down_event.empty())
	throw_event(_down_event);
      _state = DOWN;
    }
    _rgn->trap_clicks(true);
    break;
  case INACTIVE:
    if (_inactive != (GuiLabel*)0L) {
      _mgr->add_label(_inactive);
      if (!_inactive_event.empty())
	throw_event(_inactive_event);
    }
    _rgn->trap_clicks(false);
    break;
  case INACTIVE_ROLLOVER:
    if (_inactive != (GuiLabel*)0L) {
      _mgr->add_label(_inactive);
      if (!_inactive_event.empty())
	throw_event(_inactive_event);
    }
    _rgn->trap_clicks(false);
    break;
  default:
    gui_cat->warning() << "switched to invalid state (" << (int)_state << ")"
		       << endl;
  }
}

void GuiButton::recompute_frame(void) {
  GuiItem::recompute_frame();
  _up->recompute();
  _down->recompute();
  if (_up_rollover != (GuiLabel*)0L)
    _up_rollover->recompute();
  if (_down_rollover != (GuiLabel*)0L)
    _down_rollover->recompute();
  if (_inactive != (GuiLabel*)0L)
    _inactive->recompute();
  GetExtents(_up, _down, _up_rollover, _down_rollover, _inactive, _left,
	     _right, _bottom, _top);
  _rgn->set_region(_left, _right, _bottom, _top);
}

GuiButton::GuiButton(const string& name, GuiLabel* up, GuiLabel* down)
  : GuiItem(name), _up(up), _up_rollover((GuiLabel*)0L), _down(down),
    _down_rollover((GuiLabel*)0L), _inactive((GuiLabel*)0L),
    _up_event(name + "-up"), _up_rollover_event(""),
    _down_event(name +"-down"), _down_rollover_event(""),
    _inactive_event(""), _up_scale(up->get_scale()), _upr_scale(1.),
    _down_scale(down->get_scale()), _downr_scale(1.), _inactive_scale(1.),
    _state(GuiButton::NONE) {
  GetExtents(up, down, _up_rollover, _down_rollover, _inactive, _left, _right,
	     _bottom, _top);
  _rgn = new GuiRegion("button-" + name, _left, _right, _bottom, _top, true);
  buttons["gui-in-button-" + name] = this;
  buttons["gui-out-button-" + name] = this;
  buttons["gui-button-" + name + "-mouse1"] = this;
  buttons["gui-button-" + name + "-mouse2"] = this;
  buttons["gui-button-" + name + "-mouse3"] = this;
}

GuiButton::GuiButton(const string& name, GuiLabel* up, GuiLabel* down,
		     GuiLabel* inactive)
  : GuiItem(name), _up(up), _up_rollover((GuiLabel*)0L), _down(down),
    _down_rollover((GuiLabel*)0L), _inactive(inactive),
    _up_event(name + "-up"), _up_rollover_event(""),
    _down_event(name +"-down"), _down_rollover_event(""),
    _inactive_event(name + "-inactive"), _up_scale(up->get_scale()),
    _upr_scale(1.), _down_scale(down->get_scale()), _downr_scale(1.),
    _inactive_scale(inactive->get_scale()), _state(GuiButton::NONE) {
  GetExtents(up, down, _up_rollover, _down_rollover, inactive, _left, _right,
	     _bottom, _top);
  _rgn = new GuiRegion("button-" + name, _left, _right, _bottom, _top, true);
  buttons["gui-in-button-" + name] = this;
  buttons["gui-out-button-" + name] = this;
  buttons["gui-button-" + name + "-mouse1"] = this;
  buttons["gui-button-" + name + "-mouse2"] = this;
  buttons["gui-button-" + name + "-mouse3"] = this;
}

GuiButton::GuiButton(const string& name, GuiLabel* up, GuiLabel* up_roll,
		     GuiLabel* down, GuiLabel* down_roll, GuiLabel* inactive)
  : GuiItem(name), _up(up), _up_rollover(up_roll), _down(down),
    _down_rollover(down_roll), _inactive(inactive), _up_event(name + "-up"),
    _up_rollover_event(name + "-up-rollover"), _down_event(name +"-down"),
    _down_rollover_event(name + "-down-rollover"),
    _inactive_event(name + "-inactive"), _up_scale(up->get_scale()),
    _upr_scale(up_roll->get_scale()), _down_scale(down->get_scale()),
    _downr_scale(down_roll->get_scale()),
    _inactive_scale(inactive->get_scale()), _state(GuiButton::NONE) {
  GetExtents(up, down, up_roll, down_roll, inactive, _left, _right, _bottom,
	     _top);
  _rgn = new GuiRegion("button-" + name, _left, _right, _bottom, _top, true);
  buttons["gui-in-button-" + name] = this;
  buttons["gui-out-button-" + name] = this;
  buttons["gui-button-" + name + "-mouse1"] = this;
  buttons["gui-button-" + name + "-mouse2"] = this;
  buttons["gui-button-" + name + "-mouse3"] = this;
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
}

void GuiButton::manage(GuiManager* mgr, EventHandler& eh) {
  if (!_added_hooks) {
    eh.add_hook("gui-in-button-" + get_name(), enter_button);
    eh.add_hook("gui-out-button-" + get_name(), exit_button);
    eh.add_hook("gui-button-" + get_name() + "-mouse1", click_button);
    eh.add_hook("gui-button-" + get_name() + "-mouse2", click_button);
    eh.add_hook("gui-button-" + get_name() + "-mouse3", click_button);
    _added_hooks = true;
  }
  if (_mgr == (GuiManager*)0L) {
    mgr->add_region(_rgn);
    GuiItem::manage(mgr, eh);
    switch_state(UP);
  } else
    gui_cat->warning() << "tried to manage button (0x" << (void*)this
		       << ") that is already managed" << endl;
}

void GuiButton::unmanage(void) {
  if (_mgr != (GuiManager*)0L)
    _mgr->remove_region(_rgn);
  switch_state(NONE);
  GuiItem::unmanage();
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
  GuiItem::set_scale(f);
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
  GuiItem::set_pos(p);
  this->recompute_frame();
}

void GuiButton::output(ostream& os) const {
  GuiItem::output(os);
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
