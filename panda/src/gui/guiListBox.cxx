// Filename: guiListBox.cxx
// Created by:  cary (18Jan01)
// 
////////////////////////////////////////////////////////////////////

#include "guiListBox.h"

TypeHandle GuiListBox::_type_handle;

void GuiListBox::recompute_frame(void) {
  GuiItem::recompute_frame();
}

GuiListBox::GuiListBox(const string& name) : GuiItem(name) {
}

GuiListBox::~GuiListBox(void) {
  this->unmanage();
}

void GuiListBox::manage(GuiManager* mgr, EventHandler& eh) {
  if (_mgr == (GuiManager*)0L)
    GuiItem::manage(mgr, eh);
  else
    gui_cat->warning() << "tried to manage listbox (0x" << (void*)this
		       << ") that is already managed" << endl;
}

void GuiListBox::unmanage(void) {
  GuiItem::unmanage();
}

void GuiListBox::set_scale(float f) {
  GuiItem::set_scale(f);
}

void GuiListBox::set_pos(const LVector3f& p) {
  GuiItem::set_pos(p);
}

void GuiListBox::output(ostream& os) const {
  GuiItem::output(os);
  os << "  Listbox data:" << endl;
  os << "    none" << endl;
}
