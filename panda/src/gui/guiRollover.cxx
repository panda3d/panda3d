// Filename: guiRollover.cxx
// Created by:  cary (26Oct00)
// 
////////////////////////////////////////////////////////////////////

#include "guiRollover.h"
#include "config_gui.h"

#include <map>

typedef map<string, GuiRollover*> RolloverMap;
static RolloverMap rollovers;

TypeHandle GuiRollover::_type_handle;

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
  GuiRollover* val = rollovers[e->get_name()];
  val->enter();
}

static void exit_rollover(CPT_Event e) {
  GuiRollover* val = rollovers[e->get_name()];
  val->exit();
}

void GuiRollover::recompute_frame(void) {
  GuiItem::recompute_frame();
  GetExtents(_off, _on, _left, _right, _bottom, _top);
  _rgn->set_region(_left, _right, _bottom, _top);
}

GuiRollover::GuiRollover(const string& name, GuiLabel* off, GuiLabel* on)
  : GuiItem(name), _off(off), _on(on), _off_scale(off->get_scale()),
    _on_scale(on->get_scale()), _state(false) {
  GetExtents(off, on, _left, _right, _bottom, _top);
  _rgn = new GuiRegion("rollover-" + name, _left, _right, _bottom, _top,
		       false);
  rollovers["gui-in-rollover-" + name] = this;
  rollovers["gui-out-rollover-" + name] = this;
}

GuiRollover::~GuiRollover(void) {
  this->unmanage();
}

void GuiRollover::manage(GuiManager* mgr, EventHandler& eh) {
  if (!_added_hooks) {
    eh.add_hook("gui-in-rollover-" + get_name(), enter_rollover);
    eh.add_hook("gui-out-rollover-" + get_name(), exit_rollover);
    _added_hooks = true;
  }
  if (_mgr == (GuiManager*)0L) {
    mgr->add_region(_rgn);
    _state = false;
    mgr->add_label(_off);
    GuiItem::manage(mgr, eh);
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
  GuiItem::unmanage();
}

void GuiRollover::set_scale(float f) {
  _on->set_scale(f * _on_scale);
  _off->set_scale(f * _off_scale);
  GuiItem::set_scale(f);
  recompute_frame();
}

void GuiRollover::set_pos(const LVector3f& p) {
  _on->set_pos(p);
  _off->set_pos(p);
  GuiItem::set_pos(p);
  recompute_frame();
}

void GuiRollover::output(ostream& os) const {
  GuiItem::output(os);
  os << "  Rollover data:" << endl;
  os << "    off - 0x" << (void*)_off << endl;
  os << "    on - 0x" << (void*)_on << endl;
  os << "    region - 0x" << (void*)_rgn << endl;
  os << "      frame - " << _rgn->get_frame() << endl;
  os << "    state - " << _state << endl;
}
