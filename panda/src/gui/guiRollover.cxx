// Filename: guiRollover.cxx
// Created by:  cary (26Oct00)
// 
////////////////////////////////////////////////////////////////////

#include "guiRollover.h"
#include "config_gui.h"

#include <map>

typedef map<string, GuiRollover*> RolloverMap;
static RolloverMap rollovers;

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
  float left, right, bottom, top;

  GetExtents(_off, _on, left, right, bottom, top);
  _rgn->set_region(left, right, bottom, top);
}

GuiRollover::GuiRollover(const string& name, GuiLabel* off, GuiLabel* on)
  : Namable(name), _off(off), _on(on), _state(false), _added_hooks(false) {
  float left, right, bottom, top;

  GetExtents(off, on, left, right, bottom, top);
  _rgn = new GuiRegion("rollover-" + name, left, right, bottom, top, false);
  rollovers["gui-in-rollover-" + name] = this;
  rollovers["gui-out-rollover-" + name] = this;
  _mgr = (GuiManager*)0L;
}

GuiRollover::~GuiRollover(void) {
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
    _mgr = mgr;
  } else
    gui_cat->warning() << "tried to manage rollover (0x" << (void*)this
		       << ") that is already managed" << endl;
}

void GuiRollover::unmanage(void) {
  _mgr->remove_region(_rgn);
  _mgr->remove_label(_off);
  _mgr->remove_label(_on);
  _mgr = (GuiManager*)0L;
}
