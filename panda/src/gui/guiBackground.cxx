// Filename: guiBackground.cxx
// Created by:  cary (05Feb01)
// 
////////////////////////////////////////////////////////////////////

#include "guiBackground.h"
#include "config_gui.h"

TypeHandle GuiBackground::_type_handle;

void GuiBackground::recompute_frame(void) {
  _item->recompute();
  _bg->set_width(_item->get_width());
  _bg->set_height(_item->get_height());
  _bg->set_pos(LVector3f::rfu((_item->get_left() + _item->get_right())*0.5, 0.,
			      (_item->get_bottom() + _item->get_top())*0.5));
  _bg->recompute();
  GuiItem::recompute_frame();
}

void GuiBackground::set_priority(GuiLabel* l, const GuiItem::Priority p) {
  _bg->set_priority(l, ((p==P_Low)?GuiLabel::P_LOWER:GuiLabel::P_HIGHER));
}

GuiBackground::GuiBackground(const string& name, GuiItem* item)
  : GuiItem(name), _item(item) {
  _bg = GuiLabel::make_simple_card_label();
  _bg->set_width(_item->get_width());
  _bg->set_height(_item->get_height());
  _bg->set_pos(LVector3f::rfu((_item->get_left() + _item->get_right())*0.5, 0.,
			      (_item->get_bottom() + _item->get_top())*0.5));
  item->set_priority(_bg, P_High);
}

GuiBackground::GuiBackground(const string& name, GuiItem* item, Texture* tex)
  : GuiItem(name), _item(item) {
  _bg = GuiLabel::make_simple_texture_label(tex);
  _bg->set_width(_item->get_width());
  _bg->set_height(_item->get_height());
  _bg->set_pos(LVector3f::rfu((_item->get_left() + _item->get_right())*0.5, 0.,
			      (_item->get_bottom() + _item->get_top())*0.5));
  item->set_priority(_bg, P_High);
}

GuiBackground::GuiBackground(const string& name, GuiItem* item, GuiLabel* l)
  : GuiItem(name), _bg(l), _item(item) {
  _bg->set_width(_item->get_width());
  _bg->set_height(_item->get_height());
  _bg->set_pos(LVector3f::rfu((_item->get_left() + _item->get_right())*0.5, 0.,
			      (_item->get_bottom() + _item->get_top())*0.5));
  item->set_priority(_bg, P_High);
}

GuiBackground::~GuiBackground(void) {
  this->unmanage();
}

void GuiBackground::manage(GuiManager* mgr, EventHandler& eh) {
  if (!_added_hooks)
    _added_hooks = true;
  if (_mgr == (GuiManager*)0L) {
    mgr->add_label(_bg);
    _item->manage(mgr, eh);
    GuiItem::manage(mgr, eh);
  } else
    gui_cat->warning() << "tried to manage background (0x" << (void*)this
		       << ") thta is already managed" << endl;
}

void GuiBackground::manage(GuiManager* mgr, EventHandler& eh, Node* n) {
  if (!_added_hooks)
    _added_hooks = true;
  if (_mgr == (GuiManager*)0L) {
    mgr->add_label(_bg, n);
    _item->manage(mgr, eh, n);
    GuiItem::manage(mgr, eh, n);
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
  _bg->freeze();
  return _item->freeze();
}

int GuiBackground::thaw(void) {
  _bg->thaw();
  return _item->thaw();
}

void GuiBackground::set_scale(float f) {
  _bg->set_scale(f);
  _item->set_scale(f);
  GuiItem::set_scale(f);
  recompute_frame();
}

void GuiBackground::set_scale(float x, float y, float z) {
  _bg->set_scale(x, y, z);
  _item->set_scale(x, y, z);
  GuiItem::set_scale(x, y, z);
  recompute_frame();
}

void GuiBackground::set_pos(const LVector3f& p) {
  _bg->set_pos(p);
  _item->set_pos(p);
  GuiItem::set_pos(p);
  recompute_frame();
}

void GuiBackground::set_priority(GuiItem* it, const GuiItem::Priority p) {
  _item->set_priority(it, p);
  if (p == P_Highest)
    _bg->set_priority(_bg, GuiLabel::P_HIGHEST);
  else if (p == P_Lowest)
    _bg->set_priority(_bg, GuiLabel::P_LOWEST);
  else 
    it->set_priority(_bg, ((p==P_Low)?P_High:P_Low));
  GuiItem::set_priority(it, p);
}

void GuiBackground::output(ostream& os) const {
  GuiItem::output(os);
  os << "  Background data:" << endl;
  os << "    bg - 0x" << (void*)_bg << endl;
  os << "    item - 0x" << (void*)_item << endl;
  os << *_item;
}
