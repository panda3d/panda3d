// Filename: guiRollover.cxx
// Created by:  cary (26Oct00)
// 
////////////////////////////////////////////////////////////////////

#include "guiRollover.h"
#include "config_gui.h"

#include <map>

typedef map<const MouseWatcherRegion*, GuiRollover*> RolloverMap;
static RolloverMap rollovers;
static bool added_hooks = false;

TypeHandle GuiRollover::_type_handle;

static inline GuiRollover *
find_in_rollovers_map(const MouseWatcherRegion* rgn) {
  RolloverMap::iterator i = rollovers.find(rgn);
  if (i == rollovers.end())
    return (GuiRollover*)0L;
  return (*i).second;
}

inline void GetExtents(GuiLabel* x, GuiLabel* y, float& l, float& r, float& b,
		       float& t) {
  float l1, l2, r1, r2, b1, b2, t1, t2;
  x->get_extents(l1, r1, b1, t1);
  y->get_extents(l2, r2, b2, t2);
  l = (l1<l2)?l1:l2;
  r = (r1<r2)?r2:r1;
  b = (b1<b2)?b1:b2;
  t = (t1<t2)?t2:t1;
}

static void enter_rollover(CPT_Event e) {
  const MouseWatcherRegion* rgn = DCAST(MouseWatcherRegion, e->get_parameter(0).get_ptr());
  GuiRollover* val = find_in_rollovers_map(rgn);
  if (val == (GuiRollover *)0L)
    return;  // this wasn't for us
  val->enter();
}

static void exit_rollover(CPT_Event e) {
  const MouseWatcherRegion* rgn = DCAST(MouseWatcherRegion, e->get_parameter(0).get_ptr());
  GuiRollover* val = find_in_rollovers_map(rgn);
  if (val == (GuiRollover *)0L)
    return;  // this wasn't for us
  val->exit();
}

void GuiRollover::recompute_frame(void) {
  GuiBehavior::recompute_frame();
  _off->recompute();
  _on->recompute();
  this->adjust_region();
}

void GuiRollover::adjust_region(void) {
  GetExtents(_off, _on, _left, _right, _bottom, _top);
  GuiBehavior::adjust_region();
  _rgn->set_frame(_left, _right, _bottom, _top);
}

void GuiRollover::set_priority(GuiLabel* l, const GuiItem::Priority p) {
  _off->set_priority(l, ((p==P_Low)?GuiLabel::P_LOWER:GuiLabel::P_HIGHER));
  _on->set_priority(l, ((p==P_Low)?GuiLabel::P_LOWER:GuiLabel::P_HIGHER));
  GuiBehavior::set_priority(l, p);
}

GuiRollover::GuiRollover(const string& name, GuiLabel* off, GuiLabel* on)
  : GuiBehavior(name), _off(off), _on(on), _off_scale(off->get_scale()),
    _on_scale(on->get_scale()), _state(false) {
  GetExtents(off, on, _left, _right, _bottom, _top);
  _rgn = new MouseWatcherRegion("rollover-" + name, _left, _right, _bottom,
				_top);
  _rgn->set_suppress_below(false);
  rollovers[this->_rgn] = this;
}

GuiRollover::~GuiRollover(void) {
  this->unmanage();

  // Remove the names from the rollovers map, so we don't end up with
  // an invalid pointer.
  string name = get_name();
  rollovers.erase(this->_rgn);
  if ((rollovers.size() == 0) && added_hooks) {
    _eh->remove_hook("gui-enter", enter_rollover);
    _eh->remove_hook("gui-exit", exit_rollover);
    added_hooks = false;
  }
}

void GuiRollover::manage(GuiManager* mgr, EventHandler& eh) {
  if (!added_hooks) {
    eh.add_hook("gui-enter", enter_rollover);
    eh.add_hook("gui-exit", exit_rollover);
    added_hooks = true;
  }
  if (_mgr == (GuiManager*)0L) {
    mgr->add_region(_rgn);
    _state = false;
    mgr->add_label(_off);
    GuiBehavior::manage(mgr, eh);
  } else
    gui_cat->warning() << "tried to manage rollover (0x" << (void*)this
		       << ") that is already managed" << endl;
}

void GuiRollover::manage(GuiManager* mgr, EventHandler& eh, Node* n) {
  if (!added_hooks) {
    eh.add_hook("gui-enter", enter_rollover);
    eh.add_hook("gui-exit", exit_rollover);
    added_hooks = true;
  }
  if (_mgr == (GuiManager*)0L) {
    mgr->add_region(_rgn);
    _state = false;
    mgr->add_label(_off, n);
    GuiBehavior::manage(mgr, eh, n);
  } else
    gui_cat->warning() << "tried to manage rollover (0x" << (void*)this
		       << ") that is already managed" << endl;
}

void GuiRollover::unmanage(void) {
  if (_mgr != (GuiManager*)0L) {
    _mgr->remove_region(_rgn);
    _mgr->remove_label(_off);
    _mgr->remove_label(_on);
  }
  GuiBehavior::unmanage();
}

int GuiRollover::freeze(void) {
  _off->freeze();
  _on->freeze();
  return 0;
}

int GuiRollover::thaw(void) {
  _off->thaw();
  _on->thaw();
  return 0;
}

void GuiRollover::set_scale(float f) {
  _on->set_scale(f * _on_scale);
  _off->set_scale(f * _off_scale);
  GuiBehavior::set_scale(f);
  recompute_frame();
}

void GuiRollover::set_scale(float x, float y, float z) {
  _on->set_scale(x, y, z);
  _off->set_scale(x, y, z);
  GuiBehavior::set_scale(x, y, z);
  recompute_frame();
}

void GuiRollover::set_pos(const LVector3f& p) {
  _on->set_pos(p);
  _off->set_pos(p);
  GuiBehavior::set_pos(p);
  recompute_frame();
}

void GuiRollover::set_priority(GuiItem* i, const GuiItem::Priority p) {
  if (p == P_Highest) {
    _off->set_priority(_off, GuiLabel::P_HIGHEST);
    _on->set_priority(_on, GuiLabel::P_HIGHEST);
  } else if (p == P_Lowest) {
    _off->set_priority(_off, GuiLabel::P_LOWEST);
    _on->set_priority(_on, GuiLabel::P_LOWEST);
  } else {
    i->set_priority(_off, ((p==P_Low)?P_High:P_Low));
    i->set_priority(_on, ((p==P_Low)?P_High:P_Low));
  }
  GuiBehavior::set_priority(i, p);
}

void GuiRollover::start_behavior(void) {
  return;
}

void GuiRollover::stop_behavior(void) {
  return;
}

void GuiRollover::reset_behavior(void) {
  return;
}

int GuiRollover::set_draw_order(int v) {
  int o = _off->set_draw_order(v);
  o = _on->set_draw_order(o);
  _rgn->set_sort(o++);
  return GuiBehavior::set_draw_order(o);
}

void GuiRollover::output(ostream& os) const {
  GuiBehavior::output(os);
  os << "  Rollover data:" << endl;
  os << "    off - 0x" << (void*)_off << endl;
  os << "    on - 0x" << (void*)_on << endl;
  os << "    region - 0x" << (void*)_rgn << " (" << *_rgn << ")" << endl;
  os << "      frame - " << _rgn->get_frame() << endl;
  os << "    state - " << _state << endl;
}
