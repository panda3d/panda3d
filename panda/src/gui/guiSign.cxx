// Filename: guiSign.cxx
// Created by:  cary (06Nov00)
// 
////////////////////////////////////////////////////////////////////

#include "guiSign.h"
#include "config_gui.h"

void GuiSign::recompute_frame(void) {
  GuiItem::recompute_frame();
  _sign->get_extents(_left, _right, _bottom, _top);
}

GuiSign::GuiSign(const string& name, GuiLabel* sign) : GuiItem(name),
						       _sign(sign) {
  _sign->get_extents(_left, _right, _bottom, _top);
}

GuiSign::~GuiSign(void) {
}

void GuiSign::manage(GuiManager* mgr, EventHandler& eh) {
  if (!_added_hooks)
    _added_hooks = true;
  if (_mgr == (GuiManager*)0L) {
    mgr->add_label(_sign);
    GuiItem::manage(mgr, eh);
  } else
    gui_cat->warning() << "tried to manager sign (0x" << (void*)this
		       << ") that is already managed" << endl;
}

void GuiSign::unmanage(void) {
  _mgr->remove_label(_sign);
  GuiSign::unmanage();
}

void GuiSign::set_scale(float f) {
  _sign->set_scale(f);
  GuiItem::set_scale(f);
  recompute_frame();
}

void GuiSign::set_pos(const LVector3f& p) {
  _sign->set_pos(p);
  GuiItem::set_pos(p);
  recompute_frame();
}

void GuiSign::output(ostream& os) const {
  GuiItem::output(os);
  os << "  Sign data:" << endl;
  os << "    sign - 0x" << (void*)_sign << endl;
}
