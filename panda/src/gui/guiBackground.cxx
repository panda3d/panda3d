// Filename: guiBackground.cxx
// Created by:  cary (05Feb01)
// 
////////////////////////////////////////////////////////////////////

#include "guiBackground.h"
#include "config_gui.h"

TypeHandle GuiBackground::_type_handle;

void GuiBackground::recompute_frame(void) {
  GuiItem::recompute_frame();
}

GuiBackground::GuiBackground(const string& name) : GuiItem(name) {
}

GuiBackground::~GuiBackground(void) {
  this->unmanage();
}

void GuiBackground::manage(GuiManager* mgr, EventHandler& eh) {
  if (!_added_hooks)
    _added_hooks = true;
  if (_mgr == (GuiManager*)0L) {
    GuiItem::manage(mgr, eh);
  } else
    gui_cat->warning() << "tried to manage background (0x" << (void*)this
		       << ") thta is already managed" << endl;
}

void GuiBackground::unmanage(void) {
  GuiItem::unmanage();
}

int GuiBackground::freeze(void) {
  return 0;
}

int GuiBackground::thaw(void) {
  return 0;
}

void GuiBackground::set_scale(float f) {
  GuiItem::set_scale(f);
  recompute_frame();
}

void GuiBackground::set_pos(const LVector3f& p) {
  GuiItem::set_pos(p);
  recompute_frame();
}

void GuiBackground::output(ostream& os) const {
  GuiItem::output(os);
  os << "  Background data:" << endl;
  os << "    item - 0x" << (void*)0L << endl;
  // then output the item
}
