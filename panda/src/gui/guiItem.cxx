// Filename: guiItem.cxx
// Created by:  cary (01Nov00)
// 
////////////////////////////////////////////////////////////////////

#include "guiItem.h"

void GuiItem::recompute_frame(void) {
}

GuiItem::GuiItem(const string& name) : Namable(name), _added_hooks(false),
				       _mgr((GuiManager*)0L) {
}

GuiItem::~GuiItem(void) {
}

void GuiItem::manage(GuiManager* mgr, EventHandler&) {
  _mgr = mgr;
}

void GuiItem::unmanage(void) {
  _mgr = (GuiManager*)0L;
}

void GuiItem::set_scale(float f) {
  _scale = f;
}

void GuiItem::set_pos(const LVector3f& p) {
  _pos = p;
}
