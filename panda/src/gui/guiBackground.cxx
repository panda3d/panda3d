// Filename: guiBackground.cxx
// Created by:  cary (05Feb01)
// 
////////////////////////////////////////////////////////////////////

#include "guiBackground.h"
#include "config_gui.h"

TypeHandle GuiBackground::_type_handle;

void GuiBackground::recompute_frame(void) {
  _item->recompute();
  _bg->recompute();
  GuiItem::recompute_frame();
}

GuiBackground::GuiBackground(const string& name, GuiItem* item)
  : GuiItem(name), _item(item) {
  _bg = GuiLabel::make_simple_card_label();
  _bg->set_depth(0.1);
}

GuiBackground::GuiBackground(const string& name, GuiItem* item, Texture* tex)
  : GuiItem(name), _item(item) {
  _bg = GuiLabel::make_simple_texture_label(tex);
  _bg->set_depth(0.1);
}

GuiBackground::~GuiBackground(void) {
  this->unmanage();
}

void GuiBackground::manage(GuiManager* mgr, EventHandler& eh) {
  if (!_added_hooks)
    _added_hooks = true;
  if (_mgr == (GuiManager*)0L) {
    _mgr->add_label(_bg);
    _item->manage(mgr, eh);
    GuiItem::manage(mgr, eh);
  } else
    gui_cat->warning() << "tried to manage background (0x" << (void*)this
		       << ") thta is already managed" << endl;
}

void GuiBackground::unmanage(void) {
  if (_mgr != (GuiManager*)0L) {
    _mgr->remove_label(_bg);
    _item->unmanage();
  }
  GuiItem::unmanage();
}

int GuiBackground::freeze(void) {
  return _item->freeze();
}

int GuiBackground::thaw(void) {
  return _item->thaw();
}

void GuiBackground::set_scale(float f) {
  _bg->set_scale(f);
  _item->set_scale(f);
  GuiItem::set_scale(f);
  recompute_frame();
}

void GuiBackground::set_pos(const LVector3f& p) {
  _bg->set_pos(p);
  _item->set_pos(p);
  GuiItem::set_pos(p);
  recompute_frame();
}

void GuiBackground::output(ostream& os) const {
  GuiItem::output(os);
  os << "  Background data:" << endl;
  os << "    bg - 0x" << (void*)_bg << endl;
  os << "    item - 0x" << (void*)_item << endl;
  os << *_item;
}
